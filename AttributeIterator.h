
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

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
