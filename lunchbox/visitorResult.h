
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_VISITORRESULT_H
#define LUNCHBOX_VISITORRESULT_H

#include <lunchbox/types.h>
#include <iostream>

namespace lunchbox
{
/** The result code from any visit operation. */
enum VisitorResult
{
    TRAVERSE_CONTINUE,   //!< continue the traversal
    TRAVERSE_TERMINATE,  //!< abort the traversal
    TRAVERSE_PRUNE       //!< do not traverse current entity downwards
};

inline std::ostream& operator << ( std::ostream& os,
                                   const VisitorResult& result )
{
    switch( result )
    {
    case TRAVERSE_CONTINUE:  return os << "continue";
    case TRAVERSE_TERMINATE: return os << "terminate";
    case TRAVERSE_PRUNE:     return os << "prune";
    default:                 return os << "ERROR";
    }
}
}
#endif // LUNCHBOX_VISITORRESULT_H
