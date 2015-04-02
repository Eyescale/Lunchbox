
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
#include "uri.h"

namespace lunchbox
{
namespace detail
{
class PersistentMap
{
public:
    virtual ~PersistentMap() {}
    virtual size_t setQueueDepth( const size_t ) { return 0; }
    virtual bool insert( const std::string& key, const void* data,
                         const size_t size ) = 0;
    virtual std::string operator [] ( const std::string& key ) const = 0;
    virtual bool fetch(const std::string& key, char *buffer, int buflength) = 0;
    virtual bool contains( const std::string& key ) const = 0;
    virtual bool flush() = 0;
};
}
}

// Impls - need detail::PersistentMap interface above
#include "leveldb/persistentMap.h"
#include "skv/persistentMap.h"

namespace
{
lunchbox::detail::PersistentMap* _newImpl( const lunchbox::URI& uri )
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
    : _impl( _newImpl( URI( uri )))
{}

PersistentMap::PersistentMap( const URI& uri )
    : _impl( _newImpl( uri ))
{}

PersistentMap::~PersistentMap()
{
    delete _impl;
}

bool PersistentMap::handles( const URI& uri )
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
    return _impl->insert( key, data, size );
}

std::string PersistentMap::operator [] ( const std::string& key ) const
{
    return (*_impl)[ key ];
}

bool PersistentMap::fetch(const std::string& key, char *buffer, int buflength)
{
    return _impl->fetch( key, buffer, buflength );
}


bool PersistentMap::contains( const std::string& key ) const
{
    return _impl->contains( key );
}

bool PersistentMap::flush()
{
    return _impl->flush();
}

}
