
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_CHILD_ITERATOR_H
#define DASH_DETAIL_CHILD_ITERATOR_H

#include <dash/detail/Iterator.h>

namespace dash
{
namespace detail
{

template< class T > 
class ChildIterator : public Iterator< ChildIterator< T >, T, T >
{
    typedef Iterator< ChildIterator< T >, T, T > Super;
    typedef co::base::RefPtr< T > TPtr;

public:
    ChildIterator() : Super( 0, 0 ) {}
    ChildIterator( T* node, size_t i ) : Super( node, i ) {}
    template< class U >
    ChildIterator( const ChildIterator< U >& from ) : Super( from ) {}

    TPtr operator*() const { return Super::container_->getChild( Super::i_ ); }
    T* operator->() const
        { return Super::container_->getChild( Super::i_ ).get(); }
    TPtr operator[]( const size_t& n ) const
        { return Super::container_->getChild( Super::i_ + n ); }

private:
    //template< class, int32_t > friend class Child; // Child::erase
};

}
}

#endif // DASH_DETAIL_CHILD_ITERATOR_H
