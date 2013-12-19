
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_MTQUEUE_H
#define LUNCHBOX_MTQUEUE_H

#include <lunchbox/condition.h>
#include <lunchbox/debug.h>

#include <algorithm>
#include <limits.h>
#include <queue>
#include <string.h>

namespace lunchbox
{
/**
 * A thread-safe queue with a blocking read access.
 *
 * Typically used to communicate between two execution threads.
 *
 * S is deprecated by the ctor param maxSize, and defines the initial maximum
 * capacity of the Queue<T>.  When the capacity is reached, pushing new values
 * blocks until items have been consumed.
 *
 * Example: @include tests/mtQueue.cpp
 */
template< typename T, size_t S = ULONG_MAX > class MTQueue
// S = std::numeric_limits< size_t >::max() does not work:
//   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=6424
{
public:
    class Group;
    typedef T value_type;

    /** Construct a new queue. @version 1.0 */
    explicit MTQueue( const size_t maxSize = S ) : _maxSize( maxSize ) {}

    /** Construct a copy of a queue. @version 1.0 */
    MTQueue( const MTQueue< T, S >& from )  { *this = from; }

    /** Destruct this Queue. @version 1.0 */
    ~MTQueue() {}

    /** Assign the values of another queue. @version 1.0 */
    MTQueue< T, S >& operator = ( const MTQueue< T, S >& from );

    /**
     * Retrieve the requested element from the queue, may block.
     * @version 1.3.2
     */
    const T& operator[]( const size_t index ) const;

    /** @return true if the queue is empty, false otherwise. @version 1.0 */
    bool isEmpty() const { return _queue.empty(); }

    /** @return the number of items currently in the queue. @version 1.0 */
    size_t getSize() const { return _queue.size(); }

    /**
     * Set the new maximum size of the queue.
     *
     * If the new maximum size is less the current size of the queue, this
     * call will block until the queue reaches the new maximum size.
     *
     * @version 1.3.2
     */
    void setMaxSize( const size_t maxSize );

    /** @return the current maximum size of the queue. @version 1.3.2 */
    size_t getMaxSize() const { return _maxSize; }

    /**
     * Wait for the size to be at least the number of given elements.
     *
     * @return the current size when the condition was fulfilled.
     * @version 1.0
     */
    size_t waitSize( const size_t minSize ) const;

    /** Reset (empty) the queue. @version 1.0 */
    void clear();

    /**
     * Retrieve and pop the front element from the queue, may block.
     * @version 1.0
     */
    T pop();

    /**
     * Retrieve and pop the front element from the queue.
     *
     * @param timeout the timeout
     * @param element the element returned
     * @return true if an element was popped
     * @version 1.1
     */
    bool timedPop( const unsigned timeout, T& element );

    /**
     * Retrieve a number of items from the front of the queue.
     *
     * Between minimum and maximum number of items are returned in a vector. If
     * the queue has less than minimum number of elements on timeout, the result
     * vector is empty. The method returns as soon as there are at least minimum
     * elements available, i.e., it does not wait for the maximum to be reached.
     *
     * Note that this method might block up to 'minimum * timeout' milliseconds,
     * that is, the timeout defines the time to wait for an update on the queue.
     *
     * @param timeout the timeout to wait for an update
     * @param minimum the minimum number of items to retrieve
     * @param maximum the maximum number of items to retrieve
     * @return an empty vector on timeout, otherwise the result vector
     *         containing between minimum and maximum elements.
     * @version 1.7.0
     */
    std::vector< T > timedPopRange( const unsigned timeout,
                                    const size_t minimum = 1,
                                    const size_t maximum = S );

    /**
     * Retrieve and pop the front element from the queue if it is not empty.
     *
     * @param result the front value or unmodified.
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.0
     */
    bool tryPop( T& result );

    /**
     * Try to retrieve a number of items from the front of the queue.
     *
     * Between zero and the given number of items are appended to the vector.
     *
     * @param num the maximum number of items to retrieve
     * @param result the front value or unmodified.
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.1.6
     */
    void tryPop( const size_t num, std::vector< T >& result );

    /**
     * Retrieve the front element, or abort if the barrier is reached
     *
     * Used for worker threads recursively processing data, pushing it back the
     * queue. Either returns an item from the queue, or aborts if num
     * participants are waiting in the queue.
     *
     * @param result the result element, unmodified on false return value.
     * @param barrier the group's barrier handle.
     * @return true if an element was retrieved, false if the barrier height
     *         was reached.
     * @version 1.7.1
     */
    bool popBarrier( T& result, Group& barrier );

    /**
     * @param result the front value or unmodified.
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.0
     */
    bool getFront( T& result ) const;

    /**
     * @param result the last value or unmodified.
     * @return true if an element was placed in result, false if the queue
     *         is empty.
     * @version 1.0
     */
    bool getBack( T& result ) const;

    /** Push a new element to the back of the queue. @version 1.0 */
    void push( const T& element );

    /** Push a vector of elements to the back of the queue. @version 1.0 */
    void push( const std::vector< T >& elements );

    /** Push a new element to the front of the queue. @version 1.0 */
    void pushFront( const T& element );

    /** Push a vector of elements to the front of the queue. @version 1.0 */
    void pushFront( const std::vector< T >& elements );

    /** @name STL compatibility. @version 1.7.1 */
    //@{
    void push_back( const T& element ) { push( element ); }
    bool empty() const { return isEmpty(); }
    //@}

private:
    std::deque< T > _queue;
    mutable Condition _cond;
    size_t _maxSize;
};
}

#include "mtQueue.ipp" // template implementation

#endif //LUNCHBOX_MTQUEUE_H
