
/* Copyright (c) 2012-2014, Stefan Eilemann <eile@eyescale.ch>
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#ifndef LUNCHBOX_ARRAY_H
#define LUNCHBOX_ARRAY_H

#include <lunchbox/types.h>
#include <lunchbox/debug.h>

namespace lunchbox
{
/** A wrapper for C arrays without any memory management. */
template< class T > class Array
{
public:
    /** Create a new array wrapper for the given data. @version 1.0 */
    explicit Array( T* data_, const size_t num_ )
        : data( data_ ), num( num_ ) {}

    /** @return the number of bytes stored in the pointer. @version 1.0 */
    size_t getNumBytes() const { return num * sizeof( T ); }

    T* data; //!< The data
    size_t num; //!<  The number of elements in the data
};

template<> inline size_t Array< void >::getNumBytes() const { return num; }
template<> inline size_t Array< const void >::getNumBytes() const { return num;}

/** Pretty-print all members of the array. @version 1.1.1 */
template< class T >
inline std::ostream& operator << ( std::ostream& os, const Array< T >& array )
{
    return os << lunchbox::format( array.data, array.num );
}

}
#endif // LUNCHBOX_ARRAY_H
