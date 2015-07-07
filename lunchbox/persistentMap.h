
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

#ifndef LUNCHBOX_PERSISTENTMAP_H
#define LUNCHBOX_PERSISTENTMAP_H

#include <lunchbox/api.h>
#include <lunchbox/bitOperation.h> // byteswap()
#include <lunchbox/debug.h> // className
#include <lunchbox/log.h> // LBTHROW
#include <lunchbox/types.h>
#include <servus/uri.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
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
     * * skv://path_to_config\#pdsname (if LUNCHBOX_USE_SKV is defined)
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
    LUNCHBOX_API explicit PersistentMap( const servus::URI& uri );

    /** Destruct the persistent map. @version 1.9.2 */
    LUNCHBOX_API ~PersistentMap();

    /**
     * @return true if an implementation for the given URI is available.
     * @version 1.9.2
     */
    LUNCHBOX_API static bool handles( const servus::URI& uri );

    /**
     * Set the maximum number of asynchronous outstanding write operations.
     *
     * Some backend implementations support asynchronous writes, which can be
     * enabled by setting a non-zero queue depth. Applications then need to
     * quarantee that the inserted values stay valid until 'depth' other
     * elements have been inserted or flush() has been called. Implementations
     * which do not support asynchronous writes return 0.
     *
     * @return the queue depth chosen by the implementation, smaller or equal to
     *         the given depth.
     * @version 1.11
     */
    LUNCHBOX_API size_t setQueueDepth( const size_t depth );

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
     * @param values the values stored at the key.
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
     * @param values the values stored at the key.
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
     * @param key the key to retrieve.
     * @return the value, or an empty string if the key is not available.
     * @version 1.9.2
     */
    LUNCHBOX_API std::string operator [] ( const std::string& key ) const;

    /**
     * Retrieve a value for a key.
     *
     * @param key the key to retrieve.
     * @return the value, or an empty string if the key is not available.
     * @version 1.11
     */
    template< class V > V get( const std::string& key ) const
        { return _get< V >( key ); }

    /**
     * Retrieve a value as a vector for a key.
     *
     * @param key the key to retrieve.
     * @return the values, or an empty vector if the key is not available.
     * @version 1.9.2
     */
    template< class V > std::vector< V > getVector( const std::string& key )
        const;

    /**
     * Retrieve a value as a set for a key.
     *
     * @param key the key to retrieve.
     * @return the values, or an empty set if the key is not available.
     * @version 1.9.2
     */
    template< class V > std::set< V > getSet( const std::string& key ) const;

    /**
     * Asynchronously retrieve a value which to be read later.
     *
     * Might be implemented as a 'NOP' by backend implementations.
     *
     * @param key the key to retrieve.
     * @param sizeHint the size of the value, may be ignored by implementation.
     * @return false on error, true otherwise.
     * @version 1.11
     */
    LUNCHBOX_API bool fetch( const std::string& key, size_t sizeHint = 0 ) const;

    /** @return true if the key exists. @version 1.9.2 */
    LUNCHBOX_API bool contains( const std::string& key ) const;

    /** Flush outstanding operations to the backend storage. @version 1.11 */
    LUNCHBOX_API bool flush();

    /** Enable or disable endianness conversion on reads. @version 1.11 */
    LUNCHBOX_API void setByteswap( const bool swap );

private:
    detail::PersistentMap* const _impl;

    LUNCHBOX_API bool _insert( const std::string& key, const void* data,
                               const size_t size );
    LUNCHBOX_API bool _swap() const;


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

    template< class V > V _get( const std::string& k ) const
    {
        if( !boost::has_trivial_assign< V >( ))
            LBTHROW( std::runtime_error( "Can't retrieve non-POD " +
                                         className( V( ))));
        if( boost::is_pointer< V >::value )
            LBTHROW( std::runtime_error( "Can't retrieve pointers" ));

        const std::string& value = (*this)[ k ];
        if( value.size() != sizeof( V ))
            LBTHROW( std::runtime_error( std::string( "Wrong value size " ) +
                            boost::lexical_cast< std::string >( value.size( )) +
                                         " for type " + className( V( ))));

        V v( *reinterpret_cast< const V* >( value.data( )));
        if( _swap( ))
            byteswap( v );
        return v;
    }
};

template<> inline
bool PersistentMap::_insert( const std::string& k, const std::string& v,
                             const boost::false_type& )
{
    return _insert( k, v.data(), v.length( ));
}

template< class V > inline
std::vector< V > PersistentMap::getVector( const std::string& key ) const
{
    const std::string& value = (*this)[ key ];
    std::vector< V > vector( reinterpret_cast< const V* >( value.data( )),
                   reinterpret_cast< const V* >( value.data() + value.size( )));
    if( _swap() && sizeof( V ) != 1 )
    {
        BOOST_FOREACH( V& elem, vector )
            byteswap( elem );
    }
    return vector;
}

template< class V > inline
std::set< V > PersistentMap::getSet( const std::string& key ) const
{
    std::string value = (*this)[ key ];
    V* const begin = reinterpret_cast< V* >(
                         const_cast< char * >( value.data( )));
    V* const end = begin + value.size() / sizeof( V );

    if( _swap() && sizeof( V ) != 1 )
        for( V* i = begin; i < end; ++i )
            byteswap( *i );

    return std::set< V >( begin, end );
}

}

#endif //LUNCHBOX_PERSISTENTMAP_H
