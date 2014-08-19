
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

#ifndef LUNCHBOX_PERSISTENTMAP_H
#define LUNCHBOX_PERSISTENTMAP_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <boost/noncopyable.hpp>

#include <boost/type_traits.hpp>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace lunchbox
{
namespace detail { class PersistentMap; }

/**
 * Unified interface to save key-value pairs in a persistent store.
 *
 * Example: @include tests/persistentMap.cpp
 */
class PersistentMap : public boost::noncopyable
{
public:
    /**
     * Construct a new persistent map.
     *
     * Depending on the URI scheme an implementation backend is chosen. If no
     * URI is given, a default one is selected. Available implementations are:
     * * leveldb://path (if LUNCHBOX_USE_LEVELDB is defined)
     *
     * @param uri the storage backend and destination.
     * @throw std::runtime_error if no suitable implementation is found.
     * @throw leveldb::Status if opening the leveldb database failed.
     * @version 1.9.2
     */
    LUNCHBOX_API PersistentMap( const std::string& uri = std::string( ));

    /**
     * Construct a persistent map using an URI. See other ctor for details.
     * @version 1.9.2
     */
    LUNCHBOX_API explicit PersistentMap( const URI& uri );

    /** Destruct the persistent map. @version 1.9.2 */
    LUNCHBOX_API ~PersistentMap();

    /**
     * Insert or update a value in the database.
     *
     * @param key the key to store the value.
     * @param value the value stored at the key.
     * @return true on success, false otherwise
     * @version 1.9.2
     */
    template< class V > bool insert( const std::string& key, const V& value )
        { return _insert( key, &value, sizeof( value )); }

    /**
     * Insert or update a vector of values in the database.
     *
     * @param key the key to store the value.
     * @param value the values stored at the key.
     * @return true on success, false otherwise
     * @version 1.9.2
     */
    template< class V >
    bool insert( const std::string& key, const std::vector< V >& values )
        { return _insert( key, values, boost::is_pod< V >( )); }

    /**
     * Insert or update a set of values in the database.
     *
     * @param key the key to store the value.
     * @param value the values stored at the key.
     * @return true on success, false otherwise
     * @version 1.9.2
     */
    template< class V >
    bool insert( const std::string& key, const std::set< V >& values )
        { return insert( key, std::vector<V>( values.begin(), values.end( ))); }

    /**
     * Retrieve a value for a key.
     *
     * @param key the key to retreive.
     * @return the value, or an empty string if the key is not available.
     * @version 1.9.2
     */
    LUNCHBOX_API std::string operator [] ( const std::string& key ) const;

    /**
     * Retrieve a value as a vector for a key.
     *
     * @param key the key to retreive.
     * @return the values, or an empty vector if the key is not available.
     * @version 1.9.2
     */
    template< class V > std::vector< V > getVector( const std::string& key );

    /**
     * Retrieve a value as a set for a key.
     *
     * @param key the key to retreive.
     * @return the values, or an empty set if the key is not available.
     * @version 1.9.2
     */
    template< class V > std::set< V > getSet( const std::string& key );

    /** @return true if the key exists. @version 1.9.2 */
    LUNCHBOX_API bool contains( const std::string& key ) const;

private:
    detail::PersistentMap* const _impl;

    LUNCHBOX_API bool _insert( const std::string& key, const void* data,
                               const size_t size );
    template< class V >
    bool _insert( const std::string& key, const std::vector< V >& values,
                  const boost::true_type& )
        { return _insert( key, values.data(), values.size() * sizeof( V )); }
};

template< class V >
std::vector< V > PersistentMap::getVector( const std::string& key )
{
    const std::string& value = (*this)[ key ];
    return std::vector< V >( value.data(),
                             value.data() + value.size() / sizeof( V ));
}

template< class V >
std::set< V > PersistentMap::getSet( const std::string& key )
{
    const std::string& value = (*this)[ key ];
    return std::set< V >( value.data(),
                          value.data() + value.size() / sizeof( V ));
}


// inline std::ostream& operator << ( std::ostream& os, const PersistentMap& m )
}

#endif //LUNCHBOX_PERSISTENTMAP_H
