
/* Copyright (c) 2011-2013, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include "lfVectorIterator.h"

namespace lunchbox
{

template< class T, int32_t nSlots >
LFVector< T, nSlots >::LFVector()
    : size_( 0 )
{
    setZero( slots_, nSlots * sizeof( T* ));
}

template< class T, int32_t nSlots >
LFVector< T, nSlots >::LFVector( const size_t n )
    : size_( n )
{
    LBASSERT( n != 0 );
    setZero( slots_, nSlots * sizeof( T* ));
    const int32_t s = getIndexOfLastBit( uint64_t( n ));
    for( int32_t i = 0; i <= s; ++i )
        slots_[ i ] = new T[ 1<<i ];
}

template< class T, int32_t nSlots >
LFVector< T, nSlots >::LFVector( const size_t n, const T& t )
    : size_( 0 )
{
    LBASSERT( n != 0 );
    setZero( slots_, nSlots * sizeof( T* ));
    const int32_t s = getIndexOfLastBit( uint64_t( n ));
    for( int32_t i = 0; i <= s; ++i )
    {
        const size_t sz = 1<<i;
        slots_[ i ] = new T[ sz ];
        for( size_t j = 0; size_ < n && j < sz ; ++j )
        {
            slots_[ i ][ j ] = t;
            ++size_;
        }
    }
    LBASSERTINFO( size_ == n, size_ << " != " << n );
}

template< class T, int32_t nSlots >
LFVector< T, nSlots >::LFVector( const LFVector& from )
    : size_( 0 )
    , lock_()
{
    assign_( from );
}

template< class T, int32_t nSlots >
template< int32_t fromSlots >
LFVector< T, nSlots >::LFVector( const LFVector< T, fromSlots >& from )
    : size_( 0 )
    , lock_()
{
    assign_( from );
}

template< class T, int32_t nSlots >
LFVector< T, nSlots >::~LFVector()
{
    for( size_t i=0; i < nSlots; ++i )
        delete [] slots_[i];
}

template< class T, int32_t nSlots > LFVector< T, nSlots >&
LFVector< T, nSlots >::operator = ( const LFVector< T, nSlots >& from )
{
    if( &from == this )
        return *this;

    ScopedWrite mutex1( lock_ ); // DEADLOCK when doing a=b and b=a
    ScopedWrite mutex2( from.lock_ ); // consider trySet/yield approach
    size_ = 0;
    for( int32_t i = 0; i < nSlots; ++i )
    {
        if( from.slots_[i] )
        {
            const size_t sz = 1<<i;
            if( !slots_[ i ] )
                slots_[ i ] = new T[ sz ];

            for( size_t j = 0; size_ < from.size_ && j < sz ; ++j )
            {
                slots_[ i ][ j ] = from.slots_[ i ][ j ];
                ++size_;
            }
        }
        else if( slots_[ i ] ) // done copying, free unneeded slots
        {
            delete [] slots_[ i ];
            slots_[ i ] = 0;
        }
    }

    LBASSERTINFO( size_ == from.size_, size_ << " != " << from.size_ );
    return *this;
}

template< class T, int32_t nSlots >
bool LFVector< T, nSlots >::operator == ( const LFVector& rhs ) const
{
    if( &rhs == this )
        return true;

    if( size() != rhs.size() )
        return false;

    const_iterator it = begin();
    const_iterator rhsIt = rhs.begin();
    for( ; it != end() && rhsIt != end(); ++it, ++rhsIt )
    {
        if( *it != *rhsIt )
            return false;
    }

    return true;
}

// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51721
#ifdef LB_GCC_4_6_OR_LATER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

template< class T, int32_t nSlots >
T& LFVector< T, nSlots >::operator[]( size_t i )
{
    // one beyond end is possible when called by erase
    LBASSERTINFO( size_ >= i, size_ << " < " << i );
    ++i;
    const int32_t slot = getIndexOfLastBit( i );
    const size_t index = i ^ ( size_t( 1 )<<slot );

    LBASSERTINFO( slot >=0 && slot < nSlots, slot );
    LBASSERT( slots_[ slot ] );
    LBASSERT( index < (1ull<<slot) );
    return slots_[ slot ][ index ];
}

template< class T, int32_t nSlots >
const T& LFVector< T, nSlots >::operator[]( size_t i ) const
{
    LBASSERTINFO( size_ > i, size_ << " <= " << i );
    ++i;
    const int32_t slot = getIndexOfLastBit( i );
    const size_t index = i ^ ( size_t( 1 )<<slot );

    LBASSERTINFO( slot >=0 && slot < nSlots, slot );
    LBASSERT( slots_[ slot ] );
    LBASSERT( index < (1ull<<slot) );
    return slots_[ slot ][ index ];
}

#ifdef LB_GCC_4_6_OR_LATER
#  pragma GCC diagnostic pop
#endif

template< class T, int32_t nSlots >
T& LFVector< T, nSlots >::front()
{
    LBASSERT( !empty( ));
    return slots_[ 0 ][ 0 ];
}

template< class T, int32_t nSlots >
T& LFVector< T, nSlots >::back()
{
    LBASSERT( !empty( ));
    return (*this)[ size_ - 1 ];
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::expand( const size_t newSize, const T& item )
{
    ScopedWrite mutex( lock_ );
    while( newSize > size( ))
        push_back_unlocked_( item );
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::push_back( const T& item, bool lock )
{
    ScopedWrite mutex( lock ? &lock_ : 0 );
    push_back_unlocked_( item );
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::pop_back()
{
    ScopedWrite mutex( lock_ );
    if( size_ == 0 )
        return;
    --size_;
    (*this)[size_] = T(); // not correct for all T? Needed to reset RefPtr
    trim_();
}

template< class T, int32_t nSlots >
bool LFVector< T, nSlots >::pop_back( T& element )
{
    ScopedWrite mutex( lock_ );
    if( size_ == 0 )
        return false;

    element = back();
    --size_;
    (*this)[size_] = T(); // not correct for all T? Needed to reset RefPtr
    trim_();
    return true;
}

template< class T, int32_t nSlots > typename LFVector< T, nSlots >::iterator
LFVector< T, nSlots >::erase( typename LFVector< T, nSlots >::iterator pos )
{
    LBASSERT( pos.container_ == this );
    if( pos.container_ != this || pos.i_ >= size_ )
        return end();

    ScopedWrite mutex( lock_ );
    --size_;
#pragma warning (push)
#pragma warning (disable: 4996) // unchecked iterators
    std::copy( pos+1, end()+1, pos );
#pragma warning (pop)
    (*this)[size_] = T(); // correct for all T? Needed to reset RefPtr
    trim_();
    return pos;
}

template< class T, int32_t nSlots > typename LFVector< T, nSlots >::iterator
LFVector< T, nSlots >::erase( const T& element )
{
    ScopedWrite mutex( lock_ );
    for( size_t i = size_; i != 0 ; --i )
    {
        if( (*this)[i-1] == element )
        {
            --size_;
            iterator pos( this, i-1 );
#pragma warning (push)
#pragma warning (disable: 4996) // unchecked iterators
            std::copy( pos+1, end()+1, pos );
#pragma warning (pop)
            (*this)[size_] = 0; // see comment in other erase
            trim_();
            return pos;
        }
    }
    return end();
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::resize( const size_t newSize, const T& value )
{
    ScopedWrite mutex( lock_ );
    while( size_ > newSize )
    {
        --size_;
        (*this)[size_] = T(); // not correct for all T? Needed to reset RefPtr
    }
    trim_();

    while( size_ < newSize )
        push_back_unlocked_( value );
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::clear()
{
    ScopedWrite mutex( lock_ );
    while( size_ > 0 )
    {
        --size_;
        (*this)[size_] = T(); // Needed to reset RefPtr
    }
    for( int32_t i = 0; i < nSlots; ++i )
    {
        delete [] slots_[ i ];
        slots_[ i ] = 0;
    }
}

template< class T, int32_t nSlots > typename LFVector< T, nSlots >::ScopedWrite
LFVector< T, nSlots >::getWriteLock()
{
    return ScopedWrite( lock_ );
}

template< class T, int32_t nSlots >
template< int32_t fromSlots >
void LFVector< T, nSlots >::assign_( const LFVector< T, fromSlots >& from )
{
    setZero( slots_, nSlots * sizeof( T* ));

    ScopedWrite mutex( from.lock_ );
    for( int32_t i = 0; i < nSlots; ++i )
    {
        if( i >= fromSlots || !from.slots_[i] ) // done copying
        {
            LBASSERTINFO( size_ == from.size_,
                          size_ << " != " << from.size_ );
            return;
        }

        const size_t sz = 1<<i;
        slots_[ i ] = new T[ sz ];
        for( size_t j = 0; size_ < from.size_ && j < sz ; ++j )
        {
            slots_[ i ][ j ] = from.slots_[ i ][ j ];
            ++size_;
        }
    }
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::push_back_unlocked_( const T& item )
{
    const size_t i = size_ + 1;
    const int32_t slot = getIndexOfLastBit( i );
    const size_t sz = ( size_t( 1 )<<slot );
    if( !slots_[ slot ] )
        slots_[ slot ] = new T[ sz ];

    const ssize_t index = i ^ sz;
    slots_[ slot ][ index ] = item;
    ++size_;
}

template< class T, int32_t nSlots >
void LFVector< T, nSlots >::trim_()
{
    const int32_t nextSlot = getIndexOfLastBit( size_+1 ) + 1;
    if( nextSlot < nSlots && slots_[ nextSlot ] )
    {
        delete [] slots_[ nextSlot ]; // delete next slot (keep a spare)
        slots_[ nextSlot ] = 0;
    }
}

template< class T, int32_t nSlots > inline typename
LFVector< T, nSlots >::const_iterator LFVector< T, nSlots >::begin() const
{
    return const_iterator( this, 0 );
}

template< class T, int32_t nSlots > inline typename
LFVector< T, nSlots >::const_iterator LFVector< T, nSlots >::end() const
{
    return const_iterator( this, size_ );
}

template< class T, int32_t nSlots > inline typename
LFVector< T, nSlots >::iterator LFVector< T, nSlots >::begin()
{
    return iterator( this, 0 );
}

template< class T, int32_t nSlots > inline typename
LFVector< T, nSlots >::iterator LFVector< T, nSlots >::end()
{
    return iterator( this, size_ );
}

/** @cond IGNORE */
template< class T, int32_t nSlots > template< class Archive >
inline void LFVector< T, nSlots >::save( Archive& ar,
                                         const unsigned int version ) const
{
    ar << size_;
    for( size_t i = 0; i < size_; ++i )
        ar << operator[](i);
}

template< class T, int32_t nSlots > template< class Archive >
inline void LFVector< T, nSlots >::load( Archive& ar,
                                         const unsigned int version )
{
    size_t newSize;
    ar >> newSize;
    expand( newSize );
    LBASSERT( size_ == newSize );

    for( size_t i = 0; i < size_; ++i )
        ar >> operator[](i);
}
/** @endcond */

template< class T >
std::ostream& operator << ( std::ostream& os, const LFVector< T >& v )
{
    os << className( &v ) << " size " << v.size() << " [ ";
    for(typename LFVector< T >::const_iterator i = v.begin(); i != v.end(); ++i)
    {
        if( i.getPosition() > 255 )
        {
            os << "... ";
            break;
        }
        os << (*i) << ' ';
    }
    return os << ']' << std::endl;
}

}
