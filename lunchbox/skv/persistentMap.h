
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

//#ifdef LUNCHBOX_USE_SKV

#include <lunchbox/compiler.h>
#include <lunchbox/futureFunction.h>
#include <lunchbox/log.h>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#define SKV_CLIENT_UNI
#define SKV_NON_MPI
#include <skv/client/skv_client.hpp>

// Note: skv api is not const-correct. Ignore all const_cast and mutable below.
namespace lunchbox
{
namespace
{
  typedef boost::tuple<skv_client_cmd_ext_hdl_t, int*, char *> PendingGetType;
  typedef std::map< std::string, PendingGetType >     PendingGetOperations;
  typedef std::deque< skv_client_cmd_ext_hdl_t >      PendingPutOperations;
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
    }

    static bool handles( const URI& uri ) { return uri.getScheme() == "skv"; }

    size_t setQueueDepth( const size_t depth ) final
    {
        _maxPendingOps     = depth;
        _pendingGetCounter = 0;
        _pendingGets.resize(depth);
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
            _pending.push_back( handle );
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

    uint64_t fetch(const std::string& key, char *buffer, int buflength)
    {
        // calling fetch on a non async map should not be allowed?
        LBCHECK( (_maxPendingOps>0) );
        // when we make an async get, we must store the buffer pointer
        // so that we can retrieve the value, also the length of the value
        // which is written by the iRetrieve call into an int so we store the pointer
        get_operation &op_data = _pendingGets[_pendingGetCounter % _maxPendingOps];
        const skv_status_t status = _client.iRetrieve(
            &_namespace,
            const_cast< char* >( key.c_str( )), key.length(),
            buffer, buflength,
            &op_data.value_size, 0, SKV_COMMAND_RIU_FLAGS_NONE, &op_data.handle );
        //
        op_data.value_buffer = buffer;
//        std::cout << "fetch handle is " << _pendingGetCounter << " skv handle is " << *((uint64_t*)op_data.handle) << std::endl;

        if( status != SKV_SUCCESS )
        {
            LBINFO << "skv fetch failed: " << skv_status_to_string( status )
                   << std::endl;
            return 0xFFFFFF;
        }
        return _pendingGetCounter++;
    }

    std::string getfetched(uint64_t handle)
    {
        get_operation &op_data = _pendingGets[handle % _maxPendingOps];
        // asking for a key when there are pending gets, check async receives
//        std::cout << "handle is " << handle << " skv handle is " << *((uint64_t*)op_data.handle) << std::endl;
        const skv_status_t status = _client.Wait( op_data.handle );
        if( status != SKV_SUCCESS ) {
            std::cout << "An error occurred in async get "
                << skv_status_to_string( status ) << std::endl;
            return "";
        }
        std::string value;
        value.assign( op_data.value_buffer, op_data.value_size);
//        std::cout << "Async get ok, length " << op_data.value_size
//            << " value " << *reinterpret_cast< const uint64_t* >( &value[0] ) << std::endl;
        return value;
    }

    bool contains( const std::string& key ) const final
    {
        std::string value;
        return _retrieve( key, value ) == SKV_SUCCESS;
    }

    bool flush() final { return _flush( 0 ); }

private:
    mutable skv_client_t        _client;
    mutable skv_pds_id_t        _namespace;
    PendingPutOperations        _pending;
    typedef struct _get_operation {
      skv_client_cmd_ext_hdl_t  handle;
      int                       value_size;
      char                     *value_buffer;
      _get_operation() : handle(0), value_size(0), value_buffer(NULL) {}
    } get_operation;
    std::vector<get_operation>  _pendingGets;
    uint64_t                    _pendingGetCounter;
    size_t                      _maxPendingOps;

    skv_status_t _retrieve( const std::string& key, std::string& value ) const
    {
        static const size_t bufferSize = 65536;
        char buffer[ bufferSize ];
        int valueSize = 0;

        const skv_status_t status =
            _client.Retrieve( &_namespace,
                              const_cast< char* >( key.c_str( )), key.length(),
                              buffer, bufferSize, &valueSize, 0,
                              SKV_COMMAND_RIU_FLAGS_NONE );
        value.assign( buffer, valueSize );
        return status;
    }

    bool _flush( const size_t maxPending )
    {
        while( _pending.size() > maxPending )
        {
            skv_client_cmd_ext_hdl_t handle = _pending.front();
            _pending.pop_front();
            if( _client.Wait( handle ) != SKV_SUCCESS )
                return false;
        }
        return true;
    }
};

}
}

//#endif
