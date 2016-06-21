
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
#include <lunchbox/lock.h>
#include <lunchbox/scopedMutex.h>

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
        lunchbox::ScopedWrite mutex( _lock );
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
        lunchbox::ScopedWrite mutex( _lock );
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

    bool flush() final { /*NOP?*/ return true; }

private:
    memcached_st* const _instance;
    memcached_return_t _lastError;
    mutable lunchbox::Lock _lock;
};
}
}

#endif
