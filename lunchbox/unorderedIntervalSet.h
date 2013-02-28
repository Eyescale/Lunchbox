
/* Copyright (c) 2008, Juan Hernando <jhernando@fi.upm.es>
 *               2013, Daniel Nachbaur <danielnachbaur@epfl.ch>
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

#ifndef LUNCHBOX_UNORDEREDINTERVALSET_H
#define LUNCHBOX_UNORDEREDINTERVALSET_H

#include <lunchbox/types.h>
#include <set>

namespace lunchbox
{

/**
 * A container to store intervals of elements efficently.
 *
 * The type can be any class or typename which has the semantics of natural
 * numbers for addition and comparison operations. The intervals are stored in
 * an unordered fashion. Not thread-safe.
 */
template< typename T > class UnorderedIntervalSet
{
public:
    /** Element iterator which points to a current element of type T. */
    class const_iterator;

    /** Construct a new interval set. @version 1.7.1 */
    UnorderedIntervalSet();

    /** Insert a new element. @version 1.7.1 */
    void insert( const T& element );

    /** Insert a new closed interval of elements. @version 1.7.1 */
    void insert( const T& startElement, const T& endElement );

    /** Insert another interval set into this. @version 1.7.1 */
    void insert( const UnorderedIntervalSet& rhs );

    /** Remove the given element. @version 1.7.1 */
    void erase( const T& element );

    /** Remove all element inside the given closed interval. @version 1.7.1 */
    void erase( const T& startElement, const T& endElement );

    /** Remove all stored elements. @version 1.7.1 */
    void clear();

    /** Swap this container with another one. @version 1.7.1 */
    void swap( UnorderedIntervalSet& rhs );

    /** @return true if element exists. @version 1.7.1 */
    bool exists( const T& element ) const;

    /**
     * @return an iterator to element if stored, end() otherwise.
     * @version 1.7.1
     */
    const_iterator find( const T& element ) const;

    /** @return an iterator to the first element. @version 1.7.1 */
    const_iterator begin() const;

    /** @return an iterator to the end of the interval set. @version 1.7.1 */
    const_iterator end() const;

    /** @return the number of elements in the stored intervals. @version 1.7.1*/
    size_t size() const;

    /** @return true if container is empty. @version 1.7.1 */
    bool empty() const;


private:
    typedef std::pair< T, bool > Edge;

    struct EdgeCompare
    {
        bool operator()( const Edge & p1, const Edge & p2 ) const
        {
            if( p1.first < p2.first )
                return true;
            if( p1.first == p2.first )
                return p1.second && !p2.second;
            return false;
        }
    };

    // The first element of the pair is the edge value, the second element is
    // true for the start of an interval and false for the end.
    typedef std::multiset< Edge,  EdgeCompare > EdgeSet;
    EdgeSet _intervals;
    size_t _size;
};
}

#include "unorderedIntervalSet.ipp" // template implementation

#endif // LUNCHBOX_UNORDEREDINTERVALSET_H
