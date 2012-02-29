
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_ITERATOR_H
#define DASH_DETAIL_ITERATOR_H

namespace dash
{
namespace detail
{

/**
 * Counter-based, as opposed to pointer-based, iterator for any container.
 *
 * Uses curiously recurring template pattern to return proper type from
 * assignment and other operators. Access methods are to be implemented by a
 * subclass using container_ and i_. Furthermore subclasses need to implement
 * the necessary constructors (see VectorIterator).
 */
template< class S, class C, class T > class Iterator :
        public std::iterator< std::random_access_iterator_tag, T >
{
public:
    Iterator( C* container, const size_t i ) : container_( container ), i_(i) {}
    Iterator( const S& from )
            : container_( from.container_ ), i_( from.i_ ) {}
    template< class U, class V, class W >
    Iterator( const Iterator< U, V, W >& from )
            : container_( from.container_ ), i_( from.i_ ) {}

    S& operator = ( const Iterator& rhs )
        {
            container_ = rhs.container_;
            i_ = rhs.i_;
            return *static_cast< S* >( this );
        }

    template< class U, class W > S& operator = ( const Iterator< S, U, W >& rhs )
        {
            container_ = rhs.container_;
            i_ = rhs.i_;
            return *static_cast< S* >( this );
        }

    S& operator ++() { ++i_; return *static_cast< S* >( this ); }
    S& operator --() { --i_; return *static_cast< S* >( this ); }
    S operator ++ (int) { return S( container_, i_++ ); }
    S operator -- (int) { return S( container_, i_-- ); }

    S operator + ( const size_t& n ) const 
        { return S( container_, i_+n ); }
    S& operator += ( const size_t& n )
        { i_ += n; return *static_cast< S* >( this ); }

    S operator - ( const size_t& n ) const
        { return S( container_, i_-n ); }
    S& operator -= ( const size_t& n )
        { i_ -= n; return *static_cast< S* >( this ); }

    ssize_t operator- ( const S& n ) const { return i_ - n.i_; }

    bool operator == ( const S& rhs ) const 
        { return container_ == rhs.container_ && i_ == rhs.i_; }
    bool operator != ( const S& rhs ) const
        { return container_ != rhs.container_ || i_ != rhs.i_; }
    bool operator < ( const S& rhs ) const
        { return container_ <= rhs.container_ && i_ < rhs.i_; }
    bool operator > ( const S& rhs ) const
        { return container_ >= rhs.container_ && i_ > rhs.i_; }
    bool operator <= ( const S& rhs ) const
        { return container_ <= rhs.container_ && i_ <= rhs.i_; }
    bool operator >= ( const S& rhs ) const
        { return container_ >= rhs.container_ && i_ >= rhs.i_; }

    size_t getPosition() const { return i_; }

protected:
    C* container_;
    size_t i_;
    template< class, class, class > friend class Iterator; // template copy ctor

private:
    Iterator();
};

}
}

#endif // DASH_DETAIL_ITERATOR_H
