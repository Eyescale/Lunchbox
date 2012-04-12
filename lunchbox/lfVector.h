
/* Copyright (c) 2011-2012, EFPL/Blue Brain Project
 *                          Stefan Eilemann <stefan.eilemann@epfl.ch> 
 *
 * This file is part of Lunchbox <https://github.com/BlueBrain/Lunchbox>
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

#ifndef LUNCHBOX_LFVECTOR_H
#define LUNCHBOX_LFVECTOR_H

#include <lunchbox/bitOperation.h> // used inline
#include <lunchbox/debug.h> // used inline
#include <lunchbox/scopedMutex.h> // member
#include <lunchbox/serializable.h>
#include <lunchbox/spinLock.h> // member
#include <algorithm> // used inline


#ifdef _WIN32
#  define lb_bzero( ptr, size ) memset( ptr, 0, size )
#else
#  include <strings.h>
#  define lb_bzero bzero
#endif

namespace lunchbox
{

/**
 * STL-like vector implementation providing certain thread-safety guarantees.
 *
 * All operations not modifying the vector size are lock-free and wait-free. All
 * operations modifying the vector size are serialized using a spin lock. The
 * interaction of operations is documented in the corresponding modify
 * operation.
 *
 * Undocumented methods behave like the STL implementation. The number of slots
 * (default 32) sets the maximum elements the vector may hold to
 * 2^nSlots-1. Each slot needs one pointer additional storage. Naturally it
 * should never be set higher than 64.
 *
 * Not all std::vector methods are implemented.
 */
template< class T, int32_t nSlots = 32 > class LFVector
{
    typedef ScopedFastWrite ScopedWrite;

public:
    /** @version 1.3.2 */
    LFVector() : size_( 0 ) { lb_bzero( slots_, nSlots * sizeof( T* )); }

    /** @version 1.3.2 */
    explicit LFVector( const size_t n ) : size_( n )
        {
            LBASSERT( n != 0 );
            lb_bzero( slots_, nSlots * sizeof( T* ));
            const int32_t slots = getIndexOfLastBit( uint64_t( n ));
            for( int32_t i = 0; i <= slots; ++i )
                slots_[ i ] = new T[ 1<<i ];
        }

    /** @version 1.3.2 */
    explicit LFVector( const size_t n, const T& t ) : size_( 0 )
        {
            LBASSERT( n != 0 );
            lb_bzero( slots_, nSlots * sizeof( T* ));
            const int32_t slots = getIndexOfLastBit( uint64_t( n ));
            for( int32_t i = 0; i <= slots; ++i )
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

    /** @version 1.3.2 */
    explicit LFVector( const LFVector& from )
            : size_( 0 ), lock_()
        { assign_( from ); }

    /** @version 1.3.2 */
    template< int32_t fromSlots >
    explicit LFVector( const LFVector< T, fromSlots >& from )
            : size_( 0 ), lock_()
        { assign_( from ); }

    /** @version 1.3.2 */
    ~LFVector()
        {
            for( size_t i=0; i < nSlots; ++i )
                delete [] slots_[i];
        }

    /** @version 1.3.2 */
    LFVector& operator = ( const LFVector& from )
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

    /** @version 1.3.2 */
    bool operator == ( const LFVector& rhs ) const
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

    /** @version 1.3.2 */
    bool operator != ( const LFVector& rhs ) const { return !(*this == rhs); }

    bool empty() const { return size_ == 0; } //!< @version 1.3.2
    size_t size() const { return size_; } //!< @version 1.3.2

    /** @version 1.3.2 */
    T& operator[]( size_t i )
        {
            // one beyond end is possible when called by erase
            LBASSERTINFO( size_ >= i, size_ << " < " << i );
            ++i;
            const int32_t slot = getIndexOfLastBit( i );
            const size_t index = i ^ ( size_t( 1 )<<slot );

            LBASSERT( slots_[ slot ] );
            return slots_[ slot ][ index ];
        }

    /** @version 1.3.2 */
    const T& operator[]( size_t i ) const
        {
            LBASSERTINFO( size_ > i, size_ << " <= " << i );
            ++i;
            const int32_t slot = getIndexOfLastBit( i );
            const size_t index = i ^ ( size_t( 1 )<<slot );

            LBASSERT( slots_[ slot ] );
            return slots_[ slot ][ index ];
        }

    /** @version 1.3.2 */
    T& front()
        {
            LBASSERT( !empty( ));
            return slots_[ 0 ][ 0 ];
        }

    /** @version 1.3.2 */
    T& back()
        {
            LBASSERT( !empty( ));
            return (*this)[ size_ - 1 ];
        }

    /** Iterator over the vector elements. @version 1.3.2 */
    typedef LFVectorIterator< LFVector< T >, T > iterator;
    /** Iterator over the const vector elements. @version 1.3.2 */
    typedef LFVectorIterator< const LFVector< T >, const T > const_iterator;

    const_iterator begin() const; //!< @version 1.3.2
    const_iterator end() const; //!< @version 1.3.2
    iterator begin(); //!< @version 1.3.2
    iterator end(); //!< @version 1.3.2

    /** 
     * Resize the vector to at least the given size.
     *
     * In contrast to resize(), expand() only increases the size of the vector,
     * allowing concurrent resize operations on the same vector. Completely
     * thread-save with read operations. Existing end() iterators will keep
     * pointing to the old end of the vector. The size is updated after all
     * elements have been inserted, so size() followed by a read is
     * thread-safe. In contrast to <code>while( vector.size() < newSize )
     * vector.insert( item );</code> this method's operation is atomic with
     * other writes.
     *
     * @param newSize the minimum new size.
     * @param item the element to insert.
     * @version 1.3.2
     */
    void expand( const size_t newSize, const T& item = T( ))
        {
            ScopedWrite mutex( lock_ );
            while( newSize > size( ))
                push_back_unlocked_( item );
        }

    /** 
     * Add an element to the vector.
     *
     * Completely thread-save with read operations. Existing end() iterators
     * will keep pointing to the old end of the vector. The size is updated
     * after the element is inserted, so size() followed by a read is
     * thread-safe.
     *
     * @param item the element to insert.
     * @version 1.3.2
     */
    void push_back( const T& item )
        {
            ScopedWrite mutex( lock_ );
            push_back_unlocked_( item );
        }

    /**
     * Remove the last element (STL version).
     *
     * A concurrent read on the removed item produces undefined results, in
     * particular end() and back().
     *
     * @version 1.3.2
     */
    void pop_back()
        {
            ScopedWrite mutex( lock_ );
            if( size_ == 0 )
                return;
            --size_;
            (*this)[size_] = 0; // not correct for all T? Needed to reset RefPtr
            trim_();
        }

    /**
     * Remove the last element (atomic version).
     *
     * A concurrent read on the removed item produces undefined results, in
     * particular end() and back(). The last element is assigned to the given
     * element if the vector is not empty. If the vector is empty, element is
     * not touched and false is returned. The whole operation is atomic with
     * other operations changing the size of the vector.
     *
     * @param element the item receiving the value which was stored at the end.
     * @return true if the vector was not empty, false if no item was popped.
     * @version 1.3.2
     */
    bool pop_back( T& element )
        {
            ScopedWrite mutex( lock_ );
            if( size_ == 0 )
                return false;

            element = back();
            --size_;
            (*this)[size_] = 0; // not correct for all T? Needed to reset RefPtr
            trim_();
            return true;
        }

    /** 
     * Remove an element.
     *
     * A concurrent read on the item or any following item is not thread
     * save. The vector's size is decremented first. Returns end() if the
     * element can't be removed, i.e., the iterator is past end() or not for
     * this vector.
     *
     * @param pos the element to remove
     * @return an iterator pointing to the element after the removed element, or
     *         end() if nothing was erased.
     * @version 1.3.2
     */
    iterator erase( iterator pos )
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

    /** 
     * Remove the last occurence of the given element.
     *
     * A concurrent read on the item or any following item is not thread
     * save. The vector's size is decremented first. Returns end() if the
     * element can't be removed, i.e., the vector does not contain the element.
     *
     * @param element the element to remove
     * @return an iterator pointing to the element after the removed element, or
     *         end() if nothing was erased.
     * @version 1.3.2
     */
    iterator erase( const T& element )
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

    /**
     * Clear the vector and all storage.
     *
     * Thread-safe with other write operations. By nature not thread-safe with
     * read operations.
     *
     * @version 1.3.2
     */
    void clear()
        {
            ScopedWrite mutex( lock_ );
            while( size_ > 0 )
            {
                --size_;
                (*this)[size_] = 0; // Needed to reset RefPtr
            }
            for( int32_t i = 0; i < nSlots; ++i )
            {
                delete [] slots_[ i ];
                slots_[ i ] = 0;
            }
        }

private:
    LB_SERIALIZABLE

    T* slots_[ nSlots ];
    size_t size_;
    mutable SpinLock lock_;

    template< int32_t fromSlots >
    void assign_( const LFVector< T, fromSlots >& from )
        {
            lb_bzero( slots_, nSlots * sizeof( T* ));

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

    void trim_()
        {
            const int32_t nextSlot = getIndexOfLastBit( size_+1 ) + 1;
            if( nextSlot < nSlots && slots_[ nextSlot ] )
            {
                delete [] slots_[ nextSlot ]; // delete next slot (keep a spare)
                slots_[ nextSlot ] = 0;
            }
        }

    void push_back_unlocked_( const T& item )
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
};

/** Output the vector and  up to 256 items to the ostream. @version 0.1 */
template< class T > inline
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

#include "lfVectorIterator.h"

namespace lunchbox
{

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

#ifdef LUNCHBOX_USE_BOOST_SERIALIZATION
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
#endif

}

#endif // LUNCHBOX_LFVECTOR_H
