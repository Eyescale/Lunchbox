
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2011-2012, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_BITOPERATION_H
#define LUNCHBOX_BITOPERATION_H

#include <lunchbox/compiler.h>       // GCC version
#include <lunchbox/types.h>
#include <lunchbox/uint128_t.h>

#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4985) // inconsistent decl of ceil
#    include <intrin.h>
#  pragma warning (pop)
#elif defined __xlC__
#  include <builtins.h>
#  include <byteswap.h>
#elif defined (LB_GCC_4_3_OR_OLDER) && !defined(__clang__) && !defined( __APPLE__ )
#  include <byteswap.h>
#  define LB_GCC_BSWAP_FUNCTION
#endif

namespace lunchbox
{
    /** @return the position of the last (most significant) set bit, or -1. */
    template< class T > int32_t getIndexOfLastBit( T value );

    /** Swap the byte order of the given value. @version 1.5.1 */
    template< class T > void byteswap( T& value );

    // Implementation
    template<> inline int32_t getIndexOfLastBit< uint32_t >( uint32_t value )
    {
#ifdef __APPLE__
        return ::fls( value ) - 1;
#elif defined __GNUC__
        return value ? (31 - __builtin_clz( value )) : -1;
#elif defined _MSC_VER
        unsigned long i = 0;
        return _BitScanReverse( &i, value ) ? i : -1;
#else
        int32_t count = -1;
        while( value )
        {
          ++count;
          value >>= 1;
        }
        return count;
#endif
    }

    template<> inline int32_t getIndexOfLastBit< uint64_t >( uint64_t value )
    {
#ifdef  __GNUC__
        return value ? (63 - __builtin_clzll( value )) : -1;
#elif defined _WIN64
        unsigned long i = 0;
        return _BitScanReverse64( &i, value ) ? i : -1;
#else
        int32_t count = -1;
        while( value )
        {
          ++count;
          value >>= 1;
        }
        return count;
#endif
    }

#if defined(__linux__) && defined(_LP64)
    template<> inline int32_t
    getIndexOfLastBit< unsigned long long >( unsigned long long value )
        { return getIndexOfLastBit( static_cast< uint64_t >( value )); }
#endif
#ifdef __APPLE__
#  ifdef _LP64
    template<> inline
    int32_t getIndexOfLastBit< unsigned long >( unsigned long value )
        { return getIndexOfLastBit( static_cast< uint64_t >( value )); }
#  else
    template<> inline
    int32_t getIndexOfLastBit< unsigned long >( unsigned long value )
        { return getIndexOfLastBit( static_cast< uint32_t >( value )); }
#  endif
#endif

    template<> inline void byteswap( void*& ) { /*NOP*/ }
    template<> inline void byteswap( bool&) { /*NOP*/ }
    template<> inline void byteswap( char& ) { /*NOP*/ }
    template<> inline void byteswap( signed char& ) { /*NOP*/ }
    template<> inline void byteswap( unsigned char& ) { /*NOP*/ }

    template<> inline void byteswap( uint32_t& value )
    {
#ifdef _MSC_VER
        value = _byteswap_ulong( value );
#elif defined __xlC__
        __store4r( value, &value );
#elif defined LB_GCC_BSWAP_FUNCTION
        value = bswap_32( value );
#else
        value = __builtin_bswap32( value );
#endif
    }

    template<> inline void byteswap( int32_t& value )
        { byteswap( reinterpret_cast< uint32_t& >( value )); }

    template<> inline void byteswap( float& value )
        { byteswap( reinterpret_cast< uint32_t& >( value )); }

    template<> inline void byteswap( uint16_t& value )
    {
#ifdef _MSC_VER
        value = _byteswap_ushort( value );
#elif defined __xlC__
         __store2r( value, &value );
#else
        value = (value>>8) | (value<<8);
#endif
    }

    template<> inline void byteswap( int16_t& value )
        { byteswap( reinterpret_cast< uint16_t& >( value )); }

    template<> inline void byteswap( uint64_t& value )
    {
#ifdef _MSC_VER
        value = _byteswap_uint64( value );
#elif defined __xlC__
        value = __bswap_constant_64( value );
#elif defined LB_GCC_BSWAP_FUNCTION
        value = bswap_64( value );
#else
        value = __builtin_bswap64( value );
#endif
    }

    template<> inline void byteswap( int64_t& value )
        { byteswap( reinterpret_cast< uint64_t& >( value )); }

    template<> inline void byteswap( double& value )
        { byteswap( reinterpret_cast< uint64_t& >( value )); }

    template<> inline void byteswap( uint128_t& value )
    {
        byteswap( value.high( ));
        byteswap( value.low( ));
    }

    template< typename T >
    inline void byteswap( typename std::vector< T >& value )
    {
        for( size_t i = 0; i < value.size(); ++i )
            byteswap( value[i] );
    }
}
#endif //LUNCHBOX_BITOPERATION_H
