
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
    virtual bool contains( const std::string& key ) const = 0;
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
#include "leveldb/persistentMap.h"
#include "skv/persistentMap.h"

namespace
{
lunchbox::detail::PersistentMap* _newImpl( const servus::URI& uri )
{
    // Update handles() below on any change here!
#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return new lunchbox::leveldb::PersistentMap( uri );
#endif
#ifdef LUNCHBOX_USE_SKV
    if( lunchbox::skv::PersistentMap::handles( uri ))
        return new lunchbox::skv::PersistentMap( uri );
#endif

    if( !uri.getScheme().empty( ))
        LBTHROW( std::runtime_error(
                     std::string( "No suitable implementation found for: " ) +
                         boost::lexical_cast< std::string >( uri )));

#ifdef LUNCHBOX_USE_LEVELDB
    return new lunchbox::leveldb::PersistentMap( uri );
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

bool PersistentMap::handles( const servus::URI& uri )
{
#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return true;
#endif
#ifdef LUNCHBOX_USE_SKV
    if( lunchbox::skv::PersistentMap::handles( uri ))
        return true;
#endif

    if( !uri.getScheme().empty( ))
        return false;

#ifdef LUNCHBOX_USE_LEVELDB
    return true;
#endif
    return false;
}

size_t PersistentMap::setQueueDepth( const size_t depth )
{
    return _impl->setQueueDepth( depth );
}

bool PersistentMap::_insert( const std::string& key, const void* data,
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

bool PersistentMap::contains( const std::string& key ) const
{
    return _impl->contains( key );
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
