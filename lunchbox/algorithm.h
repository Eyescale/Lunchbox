
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_ALGORITHM_H
#define LUNCHBOX_ALGORITHM_H

#include <lunchbox/compiler.h>
#include <algorithm>
#include <vector>

#ifndef __MINGW32__ // MinGW does not provide the header
#  ifdef LB_GCC_4_4_OR_LATER
#    include <parallel/algorithm>
#    define LB_USE_STD_PARALLEL
#  endif
#endif

namespace lunchbox
{
/** std::sort using parallel sorting where available @version 1.9.1 */
#ifdef LB_USE_STD_PARALLEL
using std::__parallel::sort;
#else
using std::sort;
#endif

/** Find the element in the given vector. @version 1.0 */
template< typename T > typename std::vector< T >::iterator
find( std::vector< T >& container, const T& element )
    { return std::find( container.begin(), container.end(), element ); }

/** Find the element in the given vector. @version 1.0 */
template< typename T > typename std::vector< T >::const_iterator
find( const std::vector< T >& container, const T& element )
    { return std::find( container.begin(), container.end(), element ); }

/** Find the element matching the predicate @version 1.0 */
template< typename T, typename P > typename std::vector< T >::iterator
find_if( std::vector< T >& container, const P& predicate )
    { return std::find_if( container.begin(), container.end(), predicate );}

/** Find the element matching the predicate @version 1.0 */
template< typename T, typename P > typename std::vector<T>::const_iterator
find_if( std::vector< const T >& container, const P& predicate )
    { return std::find_if( container.begin(), container.end(), predicate );}

/** Uniquely sort and eliminate duplicates in a container. @version 1.9.1 */
template< typename C > void usort( C& c )
{
    std::sort( c.begin(), c.end( ));
    c.erase( std::unique( c.begin(), c.end( )), c.end( ));
}

}

#endif // LUNCHBOX_ALGORITHM_H
