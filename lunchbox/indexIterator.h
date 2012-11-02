
/* Copyright (c) 2011-2012, EFPL/Blue Brain Project
 *                          Stefan Eilemann <stefan.eilemann@epfl.ch>
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#ifndef LUNCHBOX_INDEXITERATOR_H
#define LUNCHBOX_INDEXITERATOR_H

namespace lunchbox
{

/**
 * Counter-based, as opposed to pointer-based, iterator for any container.
 *
 * Uses curiously recurring template pattern to return proper type from
 * assignment and other operators. Access methods are to be implemented by a
 * subclass using container_ and i_. Furthermore subclasses need to implement
 * the necessary constructors (see LFVectorIterator for example).
 */
template< class S, class C, class T > class IndexIterator
    : public std::iterator< std::random_access_iterator_tag, T >
{
public:
    IndexIterator( C* container, const size_t i )
            : container_( container ), i_(i) {}
    IndexIterator( const S& from )
            : container_( from.container_ ), i_( from.i_ ) {}
    template< class U, class V, class W >
    IndexIterator( const IndexIterator< U, V, W >& from )
            : container_( from.container_ ), i_( from.i_ ) {}

    S& operator = ( const IndexIterator& rhs )
        {
            container_ = rhs.container_;
            i_ = rhs.i_;
            return *static_cast< S* >( this );
        }

    template< class U, class W >
    S& operator = ( const IndexIterator< S, U, W >& rhs )
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

    // template copy ctor
    template< class, class, class > friend class IndexIterator;

private:
    IndexIterator();
};

}

#endif // LUNCHBOX_INDEXITERATOR_H
