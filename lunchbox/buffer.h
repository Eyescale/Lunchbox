
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
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

#ifndef LUNCHBOX_BUFFER_H
#define LUNCHBOX_BUFFER_H

#include <lunchbox/debug.h>       // LBASSERT macro
#include <lunchbox/os.h>          // setZero used inline
#include <lunchbox/types.h>

#include <cstdlib>      // for malloc
#include <cstring>      // for memcpy

namespace lunchbox
{
/**
 * A simple memory buffer with some helper functions.
 *
 * std::vector does not implement optimizations for growing bitwise-movable
 * vectors, i.e., it copy-constructs each element on growth.
 *
 * This buffer just memcpy's elements, i.e., it should only be used for PODs
 * since the copy constructor or assignment operator is not called on the copied
 * elements. Primarily used for binary data, e.g., in eq::Image. The
 * implementation works like a pool, that is, data is only released when the
 * buffer is deleted or clear() is called.
 */
template< class T > class Buffer
{
public:
    /** Construct a new, empty buffer. @version 1.0 */
    Buffer() : _data( nullptr ), _size( 0 ), _maxSize( 0 ) {}

    /** Construct a new buffer of the given size. @version 1.0 */
    explicit Buffer( const uint64_t size ) : _data(0), _size(0), _maxSize(0)
        { reset( size ); }

    /** Copy constructor, copies data to new Buffer. @version 1.14 */
    explicit Buffer( const Buffer& from );

    /** Move constructor, transfers data to new Buffer. @version 1.14 */
    explicit Buffer( Buffer&& from );

    /** Destruct the buffer. @version 1.0 */
    ~Buffer() { clear(); }

    /** Flush the buffer, deleting all data. @version 1.0 */
    void clear() { if( _data ) free( _data ); _data=0; _size=0; _maxSize=0; }

    /**
     * Tighten the allocated memory to the size of the buffer.
     * @return the new pointer to the first element.
     * @version 1.0
     */
    T* pack();

    /** Assignment operator, copies data from Buffer. @version 1.0 */
    Buffer& operator = ( const Buffer& from );

    /** Move operator, transfers ownership. @version 1.14 */
    Buffer& operator = ( Buffer&& from );

    /** Direct access to the element at the given index. @version 1.0 */
    T& operator [] ( const uint64_t position )
        { LBASSERT( _size > position ); return _data[ position ]; }

    /** Direct const access to an element. @version 1.0 */
    const T& operator [] ( const uint64_t position ) const
        { LBASSERT( _size > position ); return _data[ position ]; }

    /**
     * Ensure that the buffer contains at least newSize elements.
     *
     * Existing data is retained. The size is set.
     * @return the new pointer to the first element.
     * @version 1.0
     */
    T* resize( const uint64_t newSize );

    /**
     * Ensure that the buffer contains at least newSize elements.
     *
     * Existing data is retained. The size is increased, if necessary.
     * @version 1.0
     */
    void grow( const uint64_t newSize );

    /**
     * Ensure that the buffer contains at least newSize elements.
     *
     * Existing data is preserved.
     * @return the new pointer to the first element.
     * @version 1.0
     */
    T* reserve( const uint64_t newSize );

    /**
     * Set the buffer size and malloc enough memory.
     *
     * Existing data may be deleted.
     * @return the new pointer to the first element.
     * @version 1.0
     */
    T* reset( const uint64_t newSize );

    /** Set the buffer content to 0. @version 1.9.1 */
    void setZero() { ::lunchbox::setZero( _data, _size ); }

    /** Append elements to the buffer, increasing the size. @version 1.0 */
    void append( const T* data, const uint64_t size );

    /** Append one element to the buffer. @version 1.0 */
    void append( const T& element );

    /** Replace the existing data with new data. @version 1.0 */
    void replace( const void* data, const uint64_t size );

    /** Replace the existing data. @version 1.5.1 */
    void replace( const Buffer& from ) { replace( from._data, from._size ); }

    /** Swap the buffer contents with another Buffer. @version 1.0 */
    void swap( Buffer& buffer );

    /** @return a pointer to the data. @version 1.0 */
    T* getData() { return _data; }

    /** @return a const pointer to the data. @version 1.0 */
    const T* getData() const { return _data; }

    /**
     * Set the size of the buffer without changing its allocation.
     *
     * This method only modifies the size parameter. If the current
     * allocation of the buffer is too small, it asserts, returns false and
     * does not change the size.
     * @version 1.0
     */
    bool setSize( const uint64_t size );

    /** @return the current number of elements. @version 1.0 */
    uint64_t getSize() const { return _size; }

    /** @return the current storage size. @version 1.5.1 */
    uint64_t getNumBytes() const { return _size * sizeof( T ); }

    /** @return true if the buffer is empty, false if not. @version 1.0 */
    bool isEmpty() const { return (_size==0); }

    /** @return the maximum size of the buffer. @version 1.0 */
    uint64_t getMaxSize() const { return _maxSize; }

private:
    /** A pointer to the data. */
    T* _data;

    /** The number of valid items in _data. */
    uint64_t _size;

    /** The allocation _size of the buffer. */
    uint64_t _maxSize;
};
}

#include "buffer.ipp" // template implementation

#endif //LUNCHBOX_BUFFER_H
