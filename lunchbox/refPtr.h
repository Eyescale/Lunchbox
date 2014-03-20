
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *               2012-2013, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_REFPTR_H
#define LUNCHBOX_REFPTR_H

#include <lunchbox/debug.h>

#include <iostream>
#include <stdlib.h>

namespace lunchbox
{
/**
 * A smart reference pointer, aka boost::intrusive_ptr.
 *
 * Relies on the held object to implement ref() and unref()
 * correctly. Serializable using boost.serialization.
 *
 * @deprecated Use boost::intrusive_ptr
 *
 * Example: @include tests/refPtr.cpp
 */
template< class T > class RefPtr
{
    typedef T* RefPtr::*bool_t;

public:
    /** Construct a new, empty reference pointer. @version 1.0 */
    RefPtr()                     : _ptr( 0 )         {}

    /** Construct a reference pointer from a C pointer. @version 1.0 */
    RefPtr( T* const ptr )       : _ptr( ptr )       { _ref(); }

    /** Construct a copy of a reference pointer. @version 1.0 */
    RefPtr( const RefPtr& from ) : _ptr( from._ptr ) { _ref(); }

    /**
     * Construct a copy of a reference pointer of a different type.
     * @version 1.0
     */
    template< class O > RefPtr( RefPtr< O > from )
        : _ptr( from.get( )) { _ref(); }

    /** Destruct this reference pointer. @version 1.0 */
    ~RefPtr() { _unref(); _ptr = 0; }

    /** Assign another RefPtr to this reference pointer. @version 1.0 */
    RefPtr& operator = ( const RefPtr& rhs )
    {
        if( _ptr == rhs._ptr )
            return *this;

        T* tmp = _ptr;
        _ptr = rhs._ptr;
        _ref();
        if( tmp ) tmp->unref( this );
        return *this;
    }

    /** Assign a C pointer to this reference pointer. @version 1.0 */
    RefPtr& operator = ( T* ptr )
    {
        if( _ptr == ptr )
            return *this;

        T* tmp = _ptr;
        _ptr = ptr;
        _ref();
        if( tmp ) tmp->unref( this );
        return *this;
    }

    /**
     * @return true if both reference pointers hold the same C pointer.
     * @version 1.0
     */
    bool operator == ( const RefPtr& rhs ) const
    { return ( _ptr == rhs._ptr ); }

    /**
     * @return true if both reference pointer hold different C pointer.
     * @version 1.0
     */
    bool operator != ( const RefPtr& rhs ) const
    { return ( _ptr != rhs._ptr ); }

    /**
     * @return true if a pointer is held, false otherwise.
     * @version 1.1.5
     */
    operator bool_t() const { return _ptr == 0 ? 0 : &RefPtr::_ptr; }

    /** @return true if the RefPtr is empty. @version 1.0 */
    bool operator ! () const               { return ( _ptr==0 ); }

    /**
     * @return true if the left RefPtr is smaller then the right.
     * @version 1.0
     */
    bool operator < ( const RefPtr& rhs ) const { return ( _ptr < rhs._ptr ); }

    /**
     * @return true if the right RefPtr is smaller then the left.
     * @version 1.0
     */
    bool operator > ( const RefPtr& rhs ) const { return ( _ptr > rhs._ptr ); }

    /**
     * @return true if the reference pointers holds the C pointer.
     * @version 1.0
     */
    bool operator == ( const T* ptr ) const { return ( _ptr == ptr ); }

    /**
     * @return true if the reference pointers does not hold the C pointer
     * @version 1.0
     */
    bool operator != ( const T* ptr ) const { return ( _ptr != ptr ); }

    /** Access the held object. @version 1.0 */
    T*       operator->()
    { LBASSERTINFO( _ptr, className( this )); return _ptr; }
    /** Access the held object. @version 1.0 */
    const T* operator->() const
    { LBASSERTINFO( _ptr, className( this )); return _ptr; }
    /** Access the held object. @version 1.0 */
    T&       operator*()
    { LBASSERTINFO( _ptr, className( this )); return *_ptr; }
    /** Access the held object. @version 1.0 */
    const T& operator*() const
    { LBASSERTINFO( _ptr, className( this )); return *_ptr; }

    /** @return the C pointer. @version 1.0 */
    T*       get()                { return _ptr; }
    /** @return the C pointer. @version 1.0 */
    const T* get() const          { return _ptr; }

    /** @return true if the RefPtr holds a non-0 pointer. @version 1.0 */
    bool isValid() const { return ( _ptr != 0 ); }

private:
    T* _ptr;

    /** Artificially reference the held object. */
    void _ref()   { if(_ptr) _ptr->ref( this ); }

    /** Artificially dereference the held object. */
    void _unref()
    {
        if(_ptr)
        {
#ifdef NDEBUG
            _ptr->unref( this );
#else
            if( _ptr->unref( this ))
                _ptr = 0;
#endif
        }
    }
};// LB_DEPRECATED;

/** Print the reference pointer to the given output stream. */
template< class T >
inline std::ostream& operator << ( std::ostream& os, RefPtr< T > rp )
{
    const T* p = rp.get();
    if( !p )
        return os << "RP[ 0:NULL ]";

    os << disableFlush << "RP[" << p->getRefCount() << ":" << *p << "]";
    p->printHolders( os );
    return os << enableFlush;
}

template< class T > inline std::string className( const RefPtr<T>& rp )
{ return className( rp.get( )); }
}

#include <boost/serialization/split_free.hpp>

namespace boost
{
namespace serialization
{

template< class Archive, class T >
inline void save( Archive& ar, const lunchbox::RefPtr< T >& t,
                  const unsigned int /*version*/ )
{
    const T* ptr = t.get();
    ar << ptr;
}

template< class Archive, class T >
inline void load( Archive& ar, lunchbox::RefPtr< T >& t,
                  const unsigned int /*version*/ )
{
    T* obj = 0;
    ar >> obj;
    t = obj;
}

template< class Archive, class T >
inline void serialize( Archive& ar, lunchbox::RefPtr< T >& t,
                       const unsigned int version )
{
    boost::serialization::split_free( ar, t, version );
}

}
}

#endif //LUNCHBOX_REFPTR_H
