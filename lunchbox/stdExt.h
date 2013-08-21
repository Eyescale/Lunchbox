
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file lunchbox/stdExt.h
 *
 * Include extensions to the STL and define a uniform interface to them.
 */

#ifndef LUNCHBOX_STDEXT_H
#define LUNCHBOX_STDEXT_H

#include <lunchbox/compiler.h>
#include <lunchbox/uint128_t.h>

#include <algorithm>
#include <string>
#include <vector>
#ifdef LB_GCC_4_4_OR_LATER
#  include <parallel/algorithm>
#endif

//----- Common extensions of the STL
#ifdef CXX_UNORDERED_MAP_SUPPORTED
#  define LB_STDEXT_STD
#  define LB_STDEXT_STD11
#elif defined __GNUC__
#  if defined LB_GCC_4_3_OR_LATER && !defined __INTEL_COMPILER
#    define LB_STDEXT_TR1
#  elif defined __clang__
#    define LB_STDEXT_TR1
#  else
#    define LB_STDEXT_EXT
#  endif
#elif defined _MSC_VER
#  define LB_STDEXT_MSVC
#elif defined __xlC__
#  define LB_STDEXT_TR1
#  define LB_STDEXT_TR1_BOOST
#else
#  define LB_STDEXT_STD
#endif

#ifdef LB_STDEXT_TR1
#  ifdef LB_STDEXT_TR1_BOOST
#    include <boost/tr1/functional.hpp>
#    include <boost/tr1/unordered_map.hpp>
#    include <boost/tr1/unordered_set.hpp>
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#  endif
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std::tr1;
#  define LB_STDEXT_NAMESPACE_OPEN namespace std { namespace tr1 {
#  define LB_STDEXT_NAMESPACE_CLOSE }}
#endif

#ifdef LB_STDEXT_EXT
#  include <ext/hash_map>
#  include <ext/hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = __gnu_cxx;
#  define LB_STDEXT_NAMESPACE_OPEN namespace __gnu_cxx {
#  define LB_STDEXT_NAMESPACE_CLOSE }
#endif

#ifdef LB_STDEXT_MSVC
#  include <hash_map>
#  include <hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = stdext;
#  define LB_STDEXT_NAMESPACE_OPEN namespace stdext {
#  define LB_STDEXT_NAMESPACE_CLOSE }
#endif

#ifdef LB_STDEXT_STD
#  ifdef LB_STDEXT_STD11
#    include <unordered_map>
#    include <unordered_set>
#  else
#    include <hash_map>
#    include <hash_set>
#  endif
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std;
#  define LB_STDEXT_NAMESPACE_OPEN namespace std {
#  define LB_STDEXT_NAMESPACE_CLOSE }
#endif


LB_STDEXT_NAMESPACE_OPEN

//----- Our extensions of the STL
#if defined LB_STDEXT_TR1 || defined LB_STDEXT_STD11
#  ifndef LB_HAVE_HASH_MAP
    template<class K, class T, class H = hash< K >,
             class P = std::equal_to< K >, class A = std::allocator< K > >
    class hash_map : public unordered_map< K, T, H, P, A >
    {
    };
#  endif // LB_HAVE_HASH_MAP
#  ifndef LB_HAVE_HASH_SET
    template<class T, class H = hash< T >,
             class P = std::equal_to< T >, class A = std::allocator< T > >
    class hash_set : public unordered_set< T, H, P, A >
    {
    };
#  endif // LB_HAVE_HASH_SET
#endif

#ifdef LB_STDEXT_EXT
#  ifndef LB_HAVE_STRING_HASH
    /** std::string hash function. @version 1.0 */
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& str ) const
        {
            return hash< const char* >()( str.c_str() );
        }
    };
#  endif // LB_HAVE_STRING_HASH

#  if !defined __INTEL_COMPILER
#    ifndef LB_HAVE_LONG_HASH
    /** uint64_t hash function. @version 1.0 */
    template<> struct hash< uint64_t >
    {
        size_t operator()( const uint64_t& val ) const
        {
            // OPT: tr1 does the same, however it seems suboptimal on 32 bits
            // if the lower 32 bits never change, e.g., for ObjectVersion
            return static_cast< size_t >( val );
        }
    };
#    endif
#  endif // !__INTEL_COMPILER

#  ifndef LB_HAVE_VOID_PTR_HASH
    /** void* hash functions. @version 1.0 */
    template<> struct hash< void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);
        }
    };

    template<> struct hash< const void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);
        }
    };
#  endif // LB_HAVE_VOID_PTR_HASH
#endif // LB_STDEXT_EXT

#ifdef LB_STDEXT_MSVC
#  ifndef LB_HAVE_STRING_HASH

    /** std::string hash function. @version 1.0 */
    template<> inline size_t hash_compare< std::string >::operator()
        ( const std::string& key ) const
    {
        return hash_value( key.c_str( ));
    }

#  endif

#ifdef LUNCHBOX_USE_V1_API
    template<> inline size_t hash_compare< lunchbox::UUID >::operator()
        ( const lunchbox::UUID& key ) const
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }

    template<> inline size_t hash_value( const lunchbox::UUID& key )
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }
#else
    template<> inline size_t hash_compare< lunchbox::uint128_t >::operator()
        ( const lunchbox::uint128_t& key ) const
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }

    template<> inline size_t hash_value( const lunchbox::uint128_t& key )
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }
#endif

#else // MSVC

#ifdef LUNCHBOX_USE_V1_API
    /** UUID hash function. @version 1.7.0 */
    template<> struct hash< lunchbox::UUID >
    {
        size_t operator()( const lunchbox::UUID& key ) const
            {
                return key.high() ^ key.low();
            }
    };
#else
    /** uint128_t hash function. @version 1.0 */
    template<> struct hash< lunchbox::uint128_t >
    {
        size_t operator()( const lunchbox::uint128_t& key ) const
            {
                return key.high() ^ key.low();
            }
    };
#endif

#endif //! MSVC

    /**
     * Uniquely sorts and eliminates duplicates in a STL container.
     * @version 1.0
     */
    template< typename C > void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }

#ifndef LB_STDEXT_STD
    /** @deprecated Use lunchbox::sort() */
#  ifdef LB_GCC_4_4_OR_LATER
    using __gnu_parallel::sort;
#  else
    using std::sort;
#  endif
#endif

    /** @deprecated Use lunchbox::find() */
    template< typename T > typename std::vector< T >::iterator
    find( std::vector< T >& container, const T& element )
        { return std::find( container.begin(), container.end(), element ); }

    /** @deprecated Use lunchbox::find() */
    template< typename T > typename std::vector< T >::const_iterator
    find( const std::vector< T >& container, const T& element )
        { return std::find( container.begin(), container.end(), element ); }

    /** @deprecated Use lunchbox::find() */
    template< typename T, typename P > typename std::vector< T >::iterator
    find_if( std::vector< T >& container, const P& predicate )
        { return std::find_if( container.begin(), container.end(), predicate );}

    /** @deprecated Use lunchbox::find() */
    template< typename T, typename P > typename std::vector<T>::const_iterator
    find_if( std::vector< const T >& container, const P& predicate )
        { return std::find_if( container.begin(), container.end(), predicate );}

LB_STDEXT_NAMESPACE_CLOSE


#endif // LUNCHBOX_STDEXT_H
