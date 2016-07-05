
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#ifdef LUNCHBOX_USE_LIBMEMCACHED
#include <libmemcached/memcached.h>

#include <unordered_map>

namespace lunchbox
{
namespace memcached
{
namespace
{
memcached_st* _getInstance( const servus::URI& uri )
{
    const std::string& host = uri.getHost();
    const int16_t port = uri.getPort() ? uri.getPort() : 11211;
    memcached_st* instance = memcached_create( 0 );

    if( uri.getHost().empty( ))
    {
        const char* servers = ::getenv( "MEMCACHED_SERVERS" );
        if( servers )
        {
            instance->server_failure_limit = 10;
            std::string data = servers;
            while( !data.empty( ))
            {
                const size_t comma = data.find( ',' );
                const std::string& server = data.substr( 0, comma );

                const size_t colon = server.find( ':' );
                if( colon == std::string::npos )
                    memcached_server_add( instance, server.c_str(), port );
                else
                    memcached_server_add( instance,
                                          server.substr( 0, colon ).c_str(),
                                          std::stoi( server.substr( colon+1 )));

                if( comma == std::string::npos )
                    data.clear();
                else
                    data = data.substr( comma + 1 );
            }

        }
        else
            memcached_server_add( instance, "127.0.0.1", port );

    }
    else
        memcached_server_add( instance, host.c_str(), port );

    memcached_behavior_set( instance, MEMCACHED_BEHAVIOR_NO_BLOCK, 1 );
    return instance;
}
}

// memcached has relative strict requirements on keys (no whitespace or control
// characters, max length). We therefore hash incoming keys and use their string
// representation.

class PersistentMap : public detail::PersistentMap
{
public:
    explicit PersistentMap( const servus::URI& uri )
        : _instance( _getInstance( uri ))
        , _lastError( MEMCACHED_SUCCESS )
    {
        if( !_instance )
            throw std::runtime_error( std::string( "Open of " ) +
                                      std::to_string( uri ) + " failed" );
    }

    virtual ~PersistentMap() { memcached_free( _instance ); }

    static bool handles( const servus::URI& uri )
        { return uri.getScheme() == "memcached"; }

    bool insert( const std::string& key, const void* data, const size_t size )
        final
    {
        const std::string& hash = servus::make_uint128( key ).getString();
        const memcached_return_t ret =
            memcached_set( _instance, hash.c_str(), hash.length(),
                           (const char*)data, size, (time_t)0, (uint32_t)0 );

        if( ret != MEMCACHED_SUCCESS && _lastError != ret )
        {
            LBWARN << "memcached_set failed: "
                   << memcached_strerror( _instance, ret ) << std::endl;
            _lastError = ret;
        }

        return ret == MEMCACHED_SUCCESS;
    }

    std::string operator [] ( const std::string& key ) const final
    {
        const std::string& hash = servus::make_uint128( key ).getString();
        size_t size = 0;
        uint32_t flags = 0;
        memcached_return_t ret = MEMCACHED_SUCCESS;
        char* data = memcached_get( _instance, hash.c_str(), hash.length(),
                                    &size, &flags, &ret );
        std::string value;
        if( ret == MEMCACHED_SUCCESS )
        {
            value.assign( data, data + size );
            free( data );
        }
        return value;
    }

    void takeValues( const Strings& keys, const ValueFunc& func ) const final
    {
        _multiGet( keys, func, []( memcached_result_st* fetched )
                           { return memcached_result_take_value( fetched ); } );
    }

    void getValues( const Strings& keys, const ConstValueFunc& func ) const
        final
    {
        _multiGet( keys, func, []( memcached_result_st* fetched )
                                { return memcached_result_value( fetched ); } );
    }

    bool flush() final
    {
        return memcached_flush_buffers( _instance ) == MEMCACHED_SUCCESS;
    }

private:
    template< typename F, typename T >
    void _multiGet( const Strings& keys, const F& func, const T& getFunc ) const
    {
        std::vector< const char* > keysArray;
        std::vector< size_t > keyLengths;
        keysArray.reserve( keys.size( ));
        keyLengths.reserve( keys.size( ));
        std::unordered_map< std::string, std::string > hashes;

        for( const auto& key : keys )
        {
            const std::string hash = servus::make_uint128( key ).getString();
            keysArray.push_back( hash.c_str( ));
            keyLengths.push_back( hash.length( ));
            hashes[hash] = key;
        }

        memcached_return ret = memcached_mget( _instance, keysArray.data(),
                                               keyLengths.data(),
                                               keysArray.size( ));

        memcached_result_st* fetched;
        while( (fetched = memcached_fetch_result( _instance, nullptr, &ret )) )
        {
            if( ret == MEMCACHED_SUCCESS )
            {
                const std::string key( memcached_result_key_value( fetched ),
                                       memcached_result_key_length( fetched ));
                func( hashes[key], getFunc( fetched ),
                      memcached_result_length( fetched ));
            }
            memcached_result_free( fetched );
        }
    }

    memcached_st* const _instance;
    memcached_return_t _lastError;
};
}
}

#endif
