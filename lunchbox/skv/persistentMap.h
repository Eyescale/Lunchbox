
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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
#include <boost/bind.hpp>

#define SKV_CLIENT_UNI
#define SKV_NON_MPI
#include <skv/client/skv_client.hpp>

// Note: skv api is not const-correct. Ignore all const_cast and mutable below.
namespace lunchbox
{
namespace
{
bool sync( skv_client_t* client, const skv_client_cmd_ext_hdl_t handle )
{
    return client->Wait( handle ) == SKV_SUCCESS;
}
}

namespace skv
{
class PersistentMap : public detail::PersistentMap
{
public:
    PersistentMap( const URI& uri )
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
        _client.Close( &_namespace );
        _client.Disconnect();
        _client.Finalize();
    }

    static bool handles( const URI& uri ) { return uri.getScheme() == "skv"; }

    f_bool_t insert( const std::string& key, const void* data,
                           const size_t size ) final
    {
        skv_client_cmd_ext_hdl_t handle;

        const skv_status_t status =
            _client.iInsert( &_namespace,
                            const_cast< char* >( key.c_str( )), key.length(),
                            static_cast< char* >( const_cast< void* >( data )),
                             size, 0, SKV_COMMAND_RIU_UPDATE, &handle );
        if( status != SKV_SUCCESS )
        {
            LBINFO << "skv insert failed: " << skv_status_to_string( status )
                   << std::endl;
            return makeFalseFuture();
        }

        // Create and return a lazy future on skv::Client::Wait()
        return f_bool_t( new FutureFunction< bool >(
                             boost::bind( &sync, &_client, handle )));
    }

    std::string operator [] ( const std::string& key ) const final
    {
        std::string value;
        const skv_status_t status = _retrieve( key, value );
        if( status != SKV_SUCCESS )
            LBINFO << "skv retrieve failed: " << skv_status_to_string( status )
                   << std::endl;
        return value;
    }

    bool contains( const std::string& key ) const final
    {
        std::string value;
        return _retrieve( key, value ) == SKV_SUCCESS;
    }

private:
    mutable skv_client_t _client;
    mutable skv_pds_id_t _namespace;

    skv_status_t _retrieve( const std::string& key, std::string& value ) const
    {
        value.resize( 65536 );
        int valueSize = 0;

        const skv_status_t status =
            _client.Retrieve( &_namespace,
                              const_cast< char* >( key.c_str( )), key.length(),
                              const_cast< char* >( value.c_str( )),
                              value.size(), &valueSize, 0,
                              SKV_COMMAND_RIU_FLAGS_NONE );
        value.resize( valueSize );
        return status;
    }
};

}
}

#endif
