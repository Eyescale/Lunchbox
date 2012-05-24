
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "uint128_t.h"
#include "debug.h" 
#include "md5/md5.hh"

#include <cstdlib>      // for strtoull

namespace lunchbox
{
/** Special identifier values */
const uint128_t uint128_t::ZERO;

uint128_t& uint128_t::operator = ( const std::string& from )
{
    char* next = 0;
#ifdef _MSC_VER
    _high = ::_strtoui64( from.c_str(), &next, 16 );
#else
    _high = ::strtoull( from.c_str(), &next, 16 );
#endif
    LBASSERT( next != from.c_str( ));
    if( *next == '\0' ) // short representation, high was 0
    {
        _low = _high;
        _high = 0;
    }
    else
    {
        LBASSERTINFO( *next == ':', from << ", " << next );
        ++next;
#ifdef _MSC_VER
        _low = ::_strtoui64( next, 0, 16 );
#else
        _low = ::strtoull( next, 0, 16 );
#endif
    }
    return *this;
}

uint128_t make_uint128( const char* string )
{
    MD5 md5( (unsigned char*)string );
    uint8_t* data = md5.raw_digest();

    const uint128_t 
        value( (uint64_t( data[7] )<<0) | (uint64_t( data[6] )<<8) |
               (uint64_t( data[5] )<<16) | (uint64_t( data[4] )<<24) |
               (uint64_t( data[3] )<<32) | (uint64_t( data[2] )<<40) |
               (uint64_t( data[1] )<<48) | (uint64_t( data[0] )<<56),
               (uint64_t( data[15] )<<0) | (uint64_t( data[14] )<<8) |
               (uint64_t( data[13] )<<16) | (uint64_t( data[12] )<<24) |
               (uint64_t( data[11] )<<32) | (uint64_t( data[10] )<<40) |
               (uint64_t( data[9] )<<48) | (uint64_t( data[8] )<<56) );
    delete [] data;
    return value;
}

}
