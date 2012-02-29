
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

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
