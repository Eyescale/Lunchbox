
/* Copyright (c) 2014-2015, Stefan.Eilemann@epfl.ch
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef LUNCHBOX_USE_SKV

#include <lunchbox/compiler.h>
#include <lunchbox/futureFunction.h>
#include <lunchbox/log.h>
#include <lunchbox/stdExt.h>

#define SKV_CLIENT_UNI
#define SKV_NON_MPI
#include <skv/client/skv_client.hpp>

// Note: skv api is not const-correct. Ignore all const_cast and mutable below.
namespace lunchbox
{
namespace
{
typedef std::deque< skv_client_cmd_ext_hdl_t > PendingPuts;
static const size_t bufferSize = 65536;
}

namespace skv
{
class PersistentMap : public detail::PersistentMap
{
public:
    PersistentMap( const URI& uri )
        : _maxPendingOps( 0 )
    {
#ifdef SKV_CLIENT_UNI
        skv_status_t status = _client.Init( 0, 0 );
#else
        skv_status_t status = _client.Init( 0, MPI_COMM_WORLD, 0 );
#endif
        if( status != SKV_SUCCESS )
            LBTHROW( std::runtime_error( std::string( "skv init failed: " ) +
                                         skv_status_to_string( status )));

        status = _client.Connect( uri.getPath().c_str(), 0 );
        if( status != SKV_SUCCESS )
            LBTHROW( std::runtime_error( std::string( "skv connect failed: " ) +
                                         skv_status_to_string( status )));

        status = _client.Open( const_cast< char* >( uri.getFragment().c_str( )),
                              (skv_pds_priv_t)(SKV_PDS_READ | SKV_PDS_WRITE),
                              SKV_COMMAND_OPEN_FLAGS_CREATE, &_namespace );
        if( status != SKV_SUCCESS )
            LBTHROW( std::runtime_error( std::string( "skv open failed: " ) +
                                         skv_status_to_string( status )));
    }

    virtual ~PersistentMap()
    {
        flush();
        _client.Close( &_namespace );
        _client.Disconnect();
        _client.Finalize();

        LBASSERTINFO( _reads.empty(), _reads.size() << " pending reads" );
        LBASSERT( _writes.empty( ));
    }

    static bool handles( const URI& uri ) { return uri.getScheme() == "skv"; }

    size_t setQueueDepth( const size_t depth ) final
    {
        _maxPendingOps = depth;
        LBCHECK( _flush( depth ));
        return depth;
    }

    bool insert( const std::string& key, const void* data, const size_t size )
        final
    {
        skv_client_cmd_ext_hdl_t handle;
        const bool useAsync = ( _maxPendingOps > 0 );
        const skv_status_t status = useAsync ?
            _client.iInsert( &_namespace,
                             const_cast< char* >( key.c_str( )), key.length(),
                             static_cast< char* >( const_cast< void* >( data )),
                             size, 0, SKV_COMMAND_RIU_UPDATE, &handle ) :
            _client.Insert( &_namespace,
                            const_cast< char* >( key.c_str( )), key.length(),
                            static_cast< char* >( const_cast< void* >( data )),
                            size, 0, SKV_COMMAND_RIU_UPDATE );

        if( status != SKV_SUCCESS )
        {
            LBINFO << "skv insert failed: " << skv_status_to_string( status )
                   << std::endl;
            return false;
        }

        if( useAsync )
            _writes.push_back( handle );
        return _flush( _maxPendingOps );
    }

    std::string operator [] ( const std::string& key ) const final
    {
        std::string value;
        const skv_status_t status = _retrieve( key, value );
        if( status != SKV_SUCCESS )
            value.clear();
        return value;
    }

    bool fetch( const std::string& key )
    {
        // when we make an async get, we must store the buffer pointer so that
        // we can retrieve the value, also the length of the value which is
        // written by the iRetrieve call into an int so we store the pointer
        AsyncRead& asyncRead = _reads[ key ];
        if( !asyncRead.value.empty( ))
            return true; // fetch for key already pending

        asyncRead.value.resize( bufferSize );
        const skv_status_t status =
            _client.iRetrieve( &_namespace,
                               const_cast< char* >( key.c_str( )), key.length(),
                               const_cast< char* >( asyncRead.value.c_str( )),
                               asyncRead.value.length(), &asyncRead.size, 0,
                               SKV_COMMAND_RIU_FLAGS_NONE, &asyncRead.handle );

        if( status == SKV_SUCCESS )
            return true;

        LBWARN << "skv fetch failed: " << skv_status_to_string( status )
               << std::endl;
        return false;
    }

    bool contains( const std::string& key ) const final
    {
        std::string value;
        return _retrieve( key, value ) == SKV_SUCCESS;
    }

    bool flush() final { return _flush( 0 ); }

private:
    mutable skv_client_t _client;
    mutable skv_pds_id_t _namespace;
    PendingPuts _writes;

    struct AsyncRead
    {
        AsyncRead() : handle( 0 ), size( 0 ) {}

        skv_client_cmd_ext_hdl_t  handle;
        std::string               value;
        int                       size;
    };

    typedef stde::hash_map< std::string, AsyncRead > ReadMap;
    typedef ReadMap::iterator ReadMapIter;

    mutable ReadMap _reads;
    size_t _maxPendingOps;

    skv_status_t _retrieve( const std::string& key, std::string& value ) const
    {
        ReadMapIter i = _reads.find( key );
        if( i == _reads.end( )) // no async fetch pending
        {
            char buffer[ bufferSize ];
            int valueSize = 0;
            const skv_status_t status =
                _client.Retrieve( &_namespace,
                                  const_cast< char* >( key.c_str( )),
                                  key.length(),
                                  buffer, bufferSize, &valueSize, 0,
                                  SKV_COMMAND_RIU_FLAGS_NONE );
            value.assign( buffer, valueSize );
            return status;
        }

        // asking for a key when there are pending gets, check async receives
        AsyncRead& asyncRead = i->second;
        const skv_status_t status = _client.Wait( asyncRead.handle );

        asyncRead.value.resize( asyncRead.size );
        value.swap( asyncRead.value );
        _reads.erase( i );
        return status;
    }

    bool _flush( const size_t maxPending )
    {
        while( _writes.size() > maxPending )
        {
            skv_client_cmd_ext_hdl_t handle = _writes.front();
            _writes.pop_front();
            if( _client.Wait( handle ) != SKV_SUCCESS )
                return false;
        }
        return true;
    }
};

}
}
#endif
