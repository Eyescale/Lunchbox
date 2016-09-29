
/* Copyright (c) 2008-2016, Juan Hernando <jhernando@fi.upm.es>
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
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

#include <lunchbox/debug.h>

#include <boost/iterator/iterator_facade.hpp>

/** @cond IGNORE */
namespace lunchbox
{

template< typename T >
class IntervalSet< T >::const_iterator :
    public boost::iterator_facade< typename IntervalSet< T >::const_iterator,
                                   T, std::forward_iterator_tag, T >
{
public:
    // Default constructor to an end() iterator.
    const_iterator()
    {}

private:
    friend class boost::iterator_core_access;
    friend class IntervalSet;

    typedef typename IntervalSet< T >::EdgeSet::const_iterator
                     edge_iterator;

    const_iterator( const IntervalSet& set, const edge_iterator& interval )
        : _intervalIteratorEnd( set._intervals.end( ))
        , _startEdge( interval )
    {
        if (_startEdge != _intervalIteratorEnd )
            _value = _startEdge->first;
    }

    const_iterator( const IntervalSet & set, const edge_iterator & interval,
                    const T& current )
        : _value( current )
        , _intervalIteratorEnd( set._intervals.end( ))
        , _startEdge( interval )
    {}

    void increment()
    {
        if( _startEdge == _intervalIteratorEnd )
            return;

        edge_iterator endEdge = _startEdge;
        ++endEdge;
        // Next element is inside the current interval.
        if( _value >= _startEdge->first && _value < endEdge->first )
            ++_value;
        else
        {
            ++_startEdge;
            ++_startEdge;
            if( _startEdge != _intervalIteratorEnd )
                _value = _startEdge->first;
        }
    }

    bool equal( const const_iterator& rhs ) const
    {
        return (_startEdge == _intervalIteratorEnd &&
                rhs._startEdge == _intervalIteratorEnd) ||
               (_startEdge == rhs._startEdge && _value == rhs._value );
    }

    T dereference() const
    {
        return _value;
    }

    T _value;
    edge_iterator _intervalIteratorEnd;
    edge_iterator _startEdge;
};

template < typename T > IntervalSet< T >::IntervalSet()
    : _size( 0 )
{}

template < typename T > void IntervalSet< T >::insert( const T& element )
{
    insert( element, element );
}

template < typename T > void IntervalSet< T >::erase( const T& element )
{
    erase( element, element );
}

template < typename T > void IntervalSet< T >::insert( const T& startElement,
                                                       const T& endElement )
{
    LBASSERT( startElement <= endElement );

    Edge startValue( startElement, true );
    Edge endValue( endElement, false );

    if( _intervals.empty( ))
    {
        _intervals.insert( startValue );
        _intervals.insert( endValue );
        _size = endElement - startElement + 1;
        return;
    }

    // Finding the first edge whose value is less or equal than startElement.
    typename EdgeSet::iterator previous_to_start =
            _intervals.lower_bound( startValue );
    if( previous_to_start != _intervals.end( ))
    {
        if( previous_to_start == _intervals.begin( ))
        {
            if( startValue.first < previous_to_start->first )
                previous_to_start = _intervals.end();
        }
        else
            previous_to_start--;
    }
    else
        previous_to_start = (++_intervals.rbegin()).base();

    // Adding start edge as needed.
    typename EdgeSet::iterator position;
    bool fallsInside;
    size_t overlappingPortion = 0;
    T overlappingStart = T(); // initialized to silent a warning.

    if( previous_to_start == _intervals.end( ))
    {
        // Previous element doesn't exist and there is neither any of
        // equal value.
        // We have to insert start.
        position = _intervals.insert( startValue );
        fallsInside = false;
    }
    else if( !previous_to_start->second )
    {
        // Previous element is the end of one interval.
        if( previous_to_start->first + 1 == startElement )
        {
            // The end of previous interval end is one unit less than the start
            // of this interval. Removing the edge to fuse both.
            position = previous_to_start;
            position--;
            _intervals.erase( previous_to_start );
        }
        else
            // We have to insert start.
            position = _intervals.insert( previous_to_start, startValue );
        fallsInside = false;
    }
    else
    {
        // Start has fallen inside another interval we don't have to insert it.
        position = previous_to_start;
        fallsInside = true;
        overlappingStart = startElement;
    }

    // Now we have to check where the end goes.
    ++position;
    while( position != _intervals.end() && position->first <= endElement )
    {
        // Calculating the length of a possible interval overlapping the
        // interval being inserted.
        if( fallsInside )
        {
            // Previous position was the start of an overlapping interval.
            LBASSERT( !position->second );
            overlappingPortion += position->first - overlappingStart + 1;
        }
        else
            overlappingStart = position->first;

        fallsInside = position->second;

        // Note that the post-increment is evaluated before the function call
        // So position is actually pointing to the next one before the previous
        // element is erased.
        _intervals.erase( position++ );
    }

    if( position != _intervals.end() &&
        position->second && position->first == endElement + 1 )
    {
        // The end of the interval connects with the start of the next one.
        // We remove the start of the following one and don't insert this
        // edge.
        _intervals.erase(position);
    }
    else if( !fallsInside )
    {
        // End edge is not inside a previously existing interval so we
        // have to add it.
        _intervals.insert(position, endValue);
    }
    else
        overlappingPortion += endElement - overlappingStart + 1;

    _size += size_t(endElement - startElement + 1) - overlappingPortion;
    LBASSERT( _intervals.size() % 2 == 0 );
}

template < typename T > void IntervalSet< T >::erase( const T& startElement,
                                                      const T& endElement )
{
    LBASSERT( startElement <= endElement );

    if( _intervals.empty( ))
        return;

    // Finding the first edge whose value is less or equal than startElement.
    typename EdgeSet::iterator nextToStart =
        _intervals.lower_bound( std::make_pair( startElement, true ));
    typename EdgeSet::iterator previousToStart = nextToStart;
    if( nextToStart == _intervals.begin( ))
        previousToStart = _intervals.end();
    else if( nextToStart == _intervals.end( ))
        // Nothing to remove here
        return;
    else
        --previousToStart;

    typename EdgeSet::iterator position;
    bool inside;
    T overlappingStart = 0;
    size_t overlapping_portion = 0;

    if( previousToStart != _intervals.end( ))
    {
        // startElement is greater or equal than some interval edge.
        if( previousToStart->second )
        {
            // Inserting the new end of the interval starting at
            // previous_to_start.
            position =
                _intervals.insert( std::make_pair( startElement - 1, false ));
            inside = true;
            overlappingStart = startElement;
        }
        else
        {
            position = previousToStart;
            inside = false;
        }
        ++position;
    }
    else
    {
        // startElement is less than the start of any interval.
        inside = false;
        position = _intervals.begin();
        overlappingStart = position->first;
    }
    // Position has the next edge after the last interval before the removal
    // interval.

    while( position != _intervals.end() && position->first <= endElement )
    {
        if (inside)
            overlapping_portion += position->first - overlappingStart + 1;
        else
            overlappingStart = position->first;

        inside = position->second;

        // Note that the post-increment is evaluated before the function call
        // So position is actually pointing to the next one before the previous
        // element is erased.
        _intervals.erase( position++ );
    }

    if( inside )
    {
        LBASSERT( position != _intervals.end( ));
        // End edge is not inside a previously existing interval so we
        // have to add it.
        _intervals.insert (std::make_pair( endElement + 1, true ));
        overlapping_portion += endElement - overlappingStart + 1;
    }

    _size -= overlapping_portion;
    LBASSERT( _intervals.size() % 2 == 0 );
}

template < typename T > void IntervalSet< T >::insert( const IntervalSet& rhs )
{
    for( typename EdgeSet::const_iterator i = rhs._intervals.begin();
         i != rhs._intervals.end(); ++i )
    {
        insert( i->first, i->second );
    }
}

template < typename T > void IntervalSet< T >::clear()
{
    _intervals.clear();
    _size = 0;
}

template < typename T > bool IntervalSet< T >::exists( const T& element ) const
{
    return find( element ) != end();
}

template < typename T > typename IntervalSet< T >::const_iterator
IntervalSet< T >::find( const T& element ) const
{
    if( _intervals.empty( ))
        return end();

    typename EdgeSet::const_iterator next =
        _intervals.lower_bound( std::make_pair( element, false ));
    // Note that if x equals the start edge of any interval then
    // next will be the end edge due to the use of (x, false) in the
    // search.
    if( next == _intervals.end() || next == _intervals.begin( ))
        // x cannot be inside any interval.
        return end();

    typename EdgeSet::const_iterator previous = next;
    --previous;
    if( previous->second )
        return const_iterator( *this, previous, element );
    return end();
}

template < typename T > typename IntervalSet< T >::const_iterator
IntervalSet< T >::begin() const
{
    if( _intervals.empty( ))
        return end();
    return const_iterator( *this, _intervals.begin( ));
}

template < typename T > typename IntervalSet< T >::const_iterator
IntervalSet< T >::end() const
{
    return const_iterator( *this, _intervals.end());
}

template < typename T > size_t IntervalSet< T >::size() const
{
    return _size;
}

template < typename T > bool IntervalSet< T >::empty() const
{
    return size() == 0;
}

template < typename T > void IntervalSet< T >::swap( IntervalSet& rhs )
{
    _intervals.swap( rhs._intervals );
}

}
/** @endcond */
