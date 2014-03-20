
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_LFQUEUE_H
#define LUNCHBOX_LFQUEUE_H

#include <lunchbox/atomic.h> // member
#include <lunchbox/debug.h>  // used in inline method
#include <lunchbox/thread.h> // thread-safety checks

#include <vector>

namespace lunchbox
{
/**
 * A thread-safe, lock-free queue with non-blocking access.
 *
 * Typically used for caches and non-blocking communication between two threads.
 *
 * Current implementation constraints:
 * * One reader thread
 * * One writer thread
 * * Fixed maximum size (writes may fail)
 * * Not copyable
 *
 * Example: @include tests/lfQueue.cpp
 */
template< typename T > class LFQueue : public boost::noncopyable
{
public:
    /** Construct a new queue. @version 1.0 */
    explicit LFQueue( const int32_t size )
        : _data( size + 1 ), _readPos( 0 ), _writePos( 0 ) {}

    /** Destruct this queue. @version 1.0 */
    ~LFQueue() {}

    /** @return true if the queue is empty, false otherwise. @version 1.0 */
    bool isEmpty() const { return _readPos == _writePos; }

    /** Reset (empty) the queue. @version 1.0 */
    void clear();

    /**
     * Resize and reset the queue.
     *
     * This method is not thread-safe. The queue has to be empty.
     * @version 1.0
     */
    void resize( const int32_t size );

    /**
     * Retrieve and pop the front element from the queue.
     *
     * @param result the front value or unmodified
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.0
     */
    bool pop( T& result );

    /**
     * Retrieve the front element from the queue.
     *
     * @param result the front value or unmodified
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.0
     */
    bool getFront( T& result );

    /**
     * Push a new element to the back of the queue.
     *
     * @param element the element to add.
     * @return true if the element was placed, false if the queue is full
     * @version 1.0
     */
    bool push( const T& element );

    /**
     * @return the maximum number of elements held by the queue.
     * @version 1.0
     */
    size_t getCapacity() const { return _data.size() - 1; }

private:
    std::vector< T > _data;
    a_int32_t _readPos;
    a_int32_t _writePos;

    LB_TS_VAR( _reader );
    LB_TS_VAR( _writer );
};
}

#include "lfQueue.ipp" // template implementation

#endif // LUNCHBOX_LFQUEUE_H
