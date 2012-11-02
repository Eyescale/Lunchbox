
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
public:
    typedef ScopedFastWrite ScopedWrite;

    /** @version 1.3.2 */
    LFVector();

    /** @version 1.3.2 */
    explicit LFVector( const size_t n );

    /** @version 1.3.2 */
    explicit LFVector( const size_t n, const T& t );

    /** @version 1.3.2 */
    explicit LFVector( const LFVector& from );

    /** @version 1.3.2 */
    template< int32_t fromSlots >
    explicit LFVector( const LFVector< T, fromSlots >& from );

    /** @version 1.3.2 */
    ~LFVector();

    /** @version 1.3.2 */
    LFVector& operator = ( const LFVector& from );

    /** @version 1.3.2 */
    bool operator == ( const LFVector& rhs ) const;

    /** @version 1.3.2 */
    bool operator != ( const LFVector& rhs ) const { return !(*this == rhs); }

    bool empty() const { return size_ == 0; } //!< @version 1.3.2
    size_t size() const { return size_; } //!< @version 1.3.2

    /** @version 1.3.2 */
    T& operator[]( size_t i );

    /** @version 1.3.2 */
    const T& operator[]( size_t i ) const;

    /** @version 1.3.2 */
    T& front();

    /** @version 1.3.2 */
    T& back();

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
    void expand( const size_t newSize, const T& item = T( ));

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
    void push_back( const T& item );

    /**
     * Add an element to the vector.
     *
     * The same functionality as provided by push_back(), but not thread-safe.
     * The thread safety can be guaranteed using getWriteLock().
     *
     * @param item the element to insert.
     * @version 1.5
     */
    void push_back_unlocked( const T& item );

    /**
     * Remove the last element (STL version).
     *
     * A concurrent read on the removed item produces undefined results, in
     * particular end() and back().
     *
     * @version 1.3.2
     */
    void pop_back();

    /**
     * Remove the last element (atomic version).
     *
     * A concurrent read on the removed item produces undefined results, in
     * particular end() and back(). The last element is assigned to the given
     * output element if the vector is not empty. If the vector is empty,
     * element is not touched and false is returned. The whole operation is
     * atomic with other operations changing the size of the vector.
     *
     * @param element the item receiving the value which was stored at the end.
     * @return true if the vector was not empty, false if no item was popped.
     * @version 1.3.2
     */
    bool pop_back( T& element );

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
    iterator erase( iterator pos );

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
    iterator erase( const T& element );

    /**
     * Clear the vector and all storage.
     *
     * Thread-safe with other write operations. By nature not thread-safe with
     * read operations.
     *
     * @version 1.3.2
     */
    void clear();

    /** @return the locked mutex for unlocked write operations. @version 1.5 */
    ScopedWrite getWriteLock();

private:
    LB_SERIALIZABLE

    T* slots_[ nSlots ];
    size_t size_;
    mutable SpinLock lock_;

    template< int32_t fromSlots >
    void assign_( const LFVector< T, fromSlots >& from );

    void trim_();
};

/** Output the vector and  up to 256 items to the ostream. @version 0.1 */
template< class T >
std::ostream& operator << ( std::ostream& os, const LFVector< T >& v );

}

#include "lfVector.ipp" // template implementation

#endif // LUNCHBOX_LFVECTOR_H
