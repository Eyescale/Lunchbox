
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
#include <lunchbox/debug.h> // className
#include <lunchbox/log.h> // LBTHROW
#include <lunchbox/types.h>
#include <boost/noncopyable.hpp>

#include <boost/type_traits.hpp>
#include <iostream>
#include <set>
#include <stdexcept>
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
     * @throw std::runtime_error if opening the leveldb failed.
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
     * @return true if an implementation for the given URI is available.
     * @version 1.9.2
     */
    LUNCHBOX_API static bool handles( const URI& uri );

    /**
     * Insert or update a value in the database.
     *
     * @param key the key to store the value.
     * @param value the value stored at the key.
     * @return true on success, false otherwise
     * @throw std::runtime_error if the value is not copyable
     * @version 1.9.2
     */
    template< class V > bool insert( const std::string& key, const V& value )
        { return _insert( key, value, boost::has_trivial_assign< V >( )); }

    /**
     * Insert or update a vector of values in the database.
     *
     * @param key the key to store the value.
     * @param value the values stored at the key.
     * @return true on success, false otherwise
     * @throw std::runtime_error if the vector values are not copyable
     * @version 1.9.2
     */
    template< class V >
    bool insert( const std::string& key, const std::vector< V >& values )
        { return _insert( key, values, boost::has_trivial_assign< V >( )); }

    /**
     * Insert or update a set of values in the database.
     *
     * @param key the key to store the value.
     * @param value the values stored at the key.
     * @return true on success, false otherwise
     * @throw std::runtime_error if the set values are not copyable
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


    // Enables map.insert( "foo", "bar" ); bar is a char[4]. The funny braces
    // declare v as a "const ref to array of four chars", not as a "const array
    // to four char refs". Long live Bjarne!
    template< size_t N > bool
    _insert( const std::string& k, char const (& v)[N], const boost::true_type&)
    {
        return _insert( k, (void*)v, N - 1 ); // strip '0'
    }

    template< class V >
    bool _insert( const std::string& k, const V& v, const boost::true_type& )
    {
        if( boost::is_pointer< V >::value )
            LBTHROW( std::runtime_error( "Can't insert pointers" ));
        return _insert( k, &v, sizeof( V ));
    }

    template< class V >
    bool _insert( const std::string&, const V& v, const boost::false_type& )
    { LBTHROW( std::runtime_error( "Can't insert non-POD " + className( v ))); }

    template< class V >
    bool _insert( const std::string& key, const std::vector< V >& values,
                  const boost::true_type& )
        { return _insert( key, values.data(), values.size() * sizeof( V )); }
};

template<> inline
bool PersistentMap::_insert( const std::string& k, const std::string& v,
                             const boost::false_type& )
{
    return _insert( k, v.data(), v.length( ));
}

template< class V > inline
std::vector< V > PersistentMap::getVector( const std::string& key )
{
    const std::string& value = (*this)[ key ];
    return std::vector< V >( reinterpret_cast< const V* >( value.data( )),
                   reinterpret_cast< const V* >( value.data() + value.size( )));
}

template< class V > inline
std::set< V > PersistentMap::getSet( const std::string& key )
{
    const std::string& value = (*this)[ key ];
    return std::set< V >( reinterpret_cast< const V* >( value.data( )),
                          reinterpret_cast< const V* >( value.data() +
                                                        value.size( )));
}


// inline std::ostream& operator << ( std::ostream& os, const PersistentMap& m )
}

#endif //LUNCHBOX_PERSISTENTMAP_H
