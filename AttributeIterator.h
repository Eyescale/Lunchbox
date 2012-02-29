
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

#ifndef DASH_DETAIL_ATTRIBUTE_ITERATOR_H
#define DASH_DETAIL_ATTRIBUTE_ITERATOR_H

#include <dash/detail/Iterator.h>

namespace dash
{
namespace detail
{

template< class N, class A > 
class AttributeIterator : public Iterator< AttributeIterator< N, A >, N, A >
{
    typedef Iterator< AttributeIterator< N, A >, N, A > Super;
    typedef co::base::RefPtr< A > APtr;

public:
    AttributeIterator() : Super( 0, 0 ) {}
    AttributeIterator( N* node, size_t i ) : Super( node, i ) {}
    template< class U, class V >
    AttributeIterator( const AttributeIterator< U, V >& from ) : Super( from ) {}

    APtr operator*() const { return Super::container_->getAttribute(Super::i_); }
    A* operator->() const
        { return Super::container_->getAttribute( Super::i_ ).get(); }
    APtr operator[]( const size_t& n ) const
        { return Super::container_->getAttribute( Super::i_ + n ); }

private:
    //template< class, int32_t > friend class Attribute; // Attribute::erase
};

}
}

#endif // DASH_DETAIL_ATTRIBUTE_ITERATOR_H
