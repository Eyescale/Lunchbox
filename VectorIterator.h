
/* Copyright (c) 2011, EFPL/Blue Brain Project
 *                     Stefan Eilemann <stefan.eilemann@epfl.ch> 
 *
 * This file is part of DASH <https://github.com/BlueBrain/dash>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#ifndef DASH_DETAIL_VECTOR_ITERATOR_H
#define DASH_DETAIL_VECTOR_ITERATOR_H

#include <dash/detail/Iterator.h>

namespace dash
{
namespace detail
{

template< class V, class T > 
class VectorIterator : public Iterator< VectorIterator< V, T >, V, T >
{
    typedef Iterator< VectorIterator< V, T >, V, T > Super;

public:
    VectorIterator() : Super( 0, 0 ) {}
    VectorIterator( V* vector, size_t i ) : Super( vector, i ) {}
    template< class U, class W >
    VectorIterator( const VectorIterator< U, W >& from ) : Super( from ) {}

    T& operator*() const { return (*Super::container_)[ Super::i_ ]; }
    T* operator->() const { return &(*Super::container_)[ Super::i_ ]; }
    T& operator[]( const size_t& n ) const
        { return (*Super::container_)[ Super::i_ + n ]; }

private:
    template< class, int32_t > friend class Vector; // Vector::erase
};

}
}

#endif // DASH_DETAIL_VECTOR_ITERATOR_H
