
/* Copyright (c) 2014-2016, Stefan.Eilemann@epfl.ch
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

#include "persistentMap.h"
#include <servus/uri.h>

// #define HISTOGRAM

namespace lunchbox
{
namespace detail
{
class PersistentMap
{
public:
    PersistentMap() : swap( false ) {}
    virtual ~PersistentMap() {}
    virtual size_t setQueueDepth( const size_t ) { return 0; }
    virtual bool insert( const std::string& key, const void* data,
                         const size_t size ) = 0;
    virtual std::string operator [] ( const std::string& key ) const = 0;
    virtual bool fetch( const std::string&, const size_t ) const
        { return true; }
    virtual void getValues( const Strings& keys,
                            const ConstValueFunc& func ) const = 0;
    virtual void takeValues( const Strings& keys,
                             const ValueFunc& func ) const = 0;
    virtual bool flush() = 0;

    bool swap;
#ifdef HISTOGRAM
    std::map< size_t, size_t > keys;
    std::map< size_t, size_t > values;
#endif
};
}
}

// Impls - need detail::PersistentMap interface above
#include "ceph/persistentMap.h"
#include "leveldb/persistentMap.h"
#include "memcached/persistentMap.h"

namespace
{
lunchbox::detail::PersistentMap* _newImpl( const servus::URI& uri )
{
    // Update handles() below on any change here!
#ifdef LUNCHBOX_USE_RADOS
    if( lunchbox::ceph::PersistentMap::handles( uri ))
        return new lunchbox::ceph::PersistentMap( uri );
#endif
#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return new lunchbox::leveldb::PersistentMap( uri );
#endif
#ifdef LUNCHBOX_USE_LIBMEMCACHED
    if( lunchbox::memcached::PersistentMap::handles( uri ))
        return new lunchbox::memcached::PersistentMap( uri );
#endif

    LBTHROW( std::runtime_error(
                 std::string( "No suitable implementation found for: " ) +
                     boost::lexical_cast< std::string >( uri )));
}
}

namespace lunchbox
{
PersistentMap::PersistentMap( const std::string& uri )
    : _impl( _newImpl( servus::URI( uri )))
{}

PersistentMap::PersistentMap( const servus::URI& uri )
    : _impl( _newImpl( uri ))
{}

PersistentMap::~PersistentMap()
{
#ifdef HISTOGRAM
    std::cout << std::endl << "keys" << std::endl;
    for( std::pair< size_t, size_t > i : _impl->keys )
        std::cout << i.first << ", " << i.second << std::endl;
    std::cout << std::endl << "values" << std::endl;
    for( std::pair< size_t, size_t > i : _impl->values )
        std::cout << i.first << ", " << i.second << std::endl;
#endif
    delete _impl;
}

PersistentMapPtr PersistentMap::createCache()
{
#ifdef LUNCHBOX_USE_LIBMEMCACHED
    if( ::getenv( "MEMCACHED_SERVERS" ))
        return PersistentMapPtr( new PersistentMap( "memcached://" ));
#ifdef LUNCHBOX_USE_RADOS
    if( lunchbox::ceph::PersistentMap::handles( uri ))
        return true;
#endif
#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return true;
#endif
#ifdef LUNCHBOX_USE_LEVELDB
    const char* leveldb = ::getenv( "LEVELDB_CACHE" );
    if( leveldb )
        return PersistentMapPtr( new PersistentMap(
                                     std::string( "leveldb://" ) + leveldb ));
#endif

    return PersistentMapPtr();
}

bool PersistentMap::handles( const servus::URI& uri LB_UNUSED )
{
#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return true;
#endif
#ifdef LUNCHBOX_USE_LIBMEMCACHED
    if( lunchbox::memcached::PersistentMap::handles( uri ))
        return true;
#endif
    return false;
}

size_t PersistentMap::setQueueDepth( const size_t depth )
{
    return _impl->setQueueDepth( depth );
}

bool PersistentMap::insert( const std::string& key, const void* data,
                            const size_t size )
{
#ifdef HISTOGRAM
    ++_impl->keys[ key.size() ];
    ++_impl->values[ size ];
#endif
    return _impl->insert( key, data, size );
}

std::string PersistentMap::operator [] ( const std::string& key ) const
{
    return (*_impl)[ key ];
}

bool PersistentMap::fetch( const std::string& key, const size_t sizeHint ) const
{
    return _impl->fetch( key, sizeHint );
}

void PersistentMap::getValues( const Strings& keys,
                               const ConstValueFunc& func ) const
{
    _impl->getValues( keys, func );
}

void PersistentMap::takeValues( const Strings& keys,
                                const ValueFunc& func ) const
{
    _impl->takeValues( keys, func );
}

bool PersistentMap::flush()
{
    return _impl->flush();
}

void PersistentMap::setByteswap( const bool swap )
{
    _impl->swap = swap;
}

bool PersistentMap::_swap() const
{
    return _impl->swap;
}

}
