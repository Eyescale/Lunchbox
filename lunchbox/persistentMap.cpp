
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
    virtual bool insert( const std::string& key, const std::string& value ) = 0;
    virtual std::string operator [] ( const std::string& key ) const = 0;
};
}
}

// Impls - need detail::PersistentMap interface
#include "leveldb/persistentMap.h"

namespace
{
lunchbox::detail::PersistentMap* _newImpl( const std::string& uristr )
{
    const lunchbox::URI uri( uristr );

#ifdef LUNCHBOX_USE_LEVELDB
    if( lunchbox::leveldb::PersistentMap::handles( uri ))
        return new lunchbox::leveldb::PersistentMap( uri );
#endif

    if( !uri.getScheme().empty( ))
        throw std::runtime_error(
            std::string( "No suitable implementation found for: " ) + uristr );

#ifdef LUNCHBOX_USE_LEVELDB
    return new lunchbox::leveldb::PersistentMap( uri );
#endif
    throw std::runtime_error(
        std::string( "No suitable implementation found for: " ) + uristr );
}
}

namespace lunchbox
{
PersistentMap::PersistentMap( const std::string& uri )
    : _impl( _newImpl( uri ))
{
}

PersistentMap::~PersistentMap()
{
    delete _impl;
}

bool PersistentMap::insert( const std::string& key, const std::string& value)
{
    return _impl->insert( key, value );
}

std::string PersistentMap::operator [] ( const std::string& key ) const
{
    return (*_impl)[ key ];
}

}
