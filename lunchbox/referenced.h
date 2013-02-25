
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_REFERENCED_H
#define LUNCHBOX_REFERENCED_H

#include <lunchbox/api.h>      // for LUNCHBOX_API
#include <lunchbox/atomic.h>   // member
#include <lunchbox/debug.h>    // for LBERROR

//#define LUNCHBOX_REFERENCED_DEBUG
#ifdef LUNCHBOX_REFERENCED_DEBUG
#  include <lunchbox/clock.h>
#  include <lunchbox/hash.h>
#  include <lunchbox/lockable.h>
#  include <lunchbox/scopedMutex.h>
#  include <lunchbox/spinLock.h>
#endif

namespace lunchbox
{
/**
 * Base class for referenced objects.
 *
 * Implements reference-counted objects which destroy themselves once they are
 * no longer referenced. Uses an Atomic variable to keep the reference count
 * access thread-safe and efficient.
 *
 * @sa RefPtr
 */
class Referenced
{
public:
    /** Increase the reference count. @version 1.0 .*/
    void ref( const void* holder = 0 ) const
        {
#ifndef NDEBUG
            LBASSERT( !_hasBeenDeleted );
#endif
            ++_refCount;

#ifdef LUNCHBOX_REFERENCED_DEBUG
            if( holder )
            {
                std::stringstream cs;
                cs << "Thread " << Log::instance().getThreadName() << " @ "
                   << Log::instance().getClock().getTime64() << " rc "
                   << _refCount << " from " << backtrace;
                ScopedFastWrite mutex( _holders );
                HolderHash::iterator i = _holders->find( holder );
                LBASSERTINFO( i == _holders->end(), i->second );
                _holders.data[ holder ] = cs.str();
            }
#endif
        }

    /**
     * Decrease the reference count.
     *
     * The object is deleted when the reference count reaches 0.
     * @version 1.0
     * @return true if the reference count went to 0, false otherwise.
     */
    bool unref( const void* holder = 0 ) const
        {
#ifndef NDEBUG
            LBASSERT( !_hasBeenDeleted );
#endif
            LBASSERT( _refCount > 0 );
            const bool last = (--_refCount==0);

#ifdef LUNCHBOX_REFERENCED_DEBUG
            if( holder )
            {
                ScopedFastWrite mutex( _holders );
                HolderHash::iterator i = _holders->find( holder );
                LBASSERT( i != _holders->end( ));
                _holders->erase( i );
                LBASSERT( _holders->find( holder ) == _holders->end( ));
            }
#endif

            if( last )
                const_cast< Referenced* >( this )->notifyFree();
            return last;
        }

    /** @return the current reference count. @version 1.0 */
    int32_t getRefCount() const { return _refCount; }

    /** @internal print holders of this if debugging is enabled. */
    void printHolders( std::ostream& os ) const
        {
#ifdef LUNCHBOX_REFERENCED_DEBUG
            os << disableFlush << disableHeader << std::endl;
            {
                ScopedFastRead mutex( _holders );
                for( HolderHash::const_iterator i = _holders->begin();
                     i != _holders->end(); ++i )
                {
                    os << "Holder " << i->first << ": " << i->second
                       << std::endl;
                }
            }
            os << enableHeader << enableFlush;
#endif
        }

protected:
    /** Construct a new reference-counted object. @version 1.0 */
    Referenced()
        : _refCount( 0 )
        , _hasBeenDeleted( false )
        {}

    /** Construct a new copy of a reference-counted object. @version 1.0 */
    Referenced( const Referenced& )
    : _refCount( 0 )
    , _hasBeenDeleted( false )
        {}

    /** Destruct a reference-counted object. @version 1.0 */
    virtual ~Referenced()
        {
#ifndef NDEBUG
            LBASSERT( !_hasBeenDeleted );
            _hasBeenDeleted = true;
#endif
            LBASSERTINFO( _refCount == 0,
                          "Deleting object with ref count " << _refCount );
        }

    /** Assign another object to this object. @version 1.1.3 */
    Referenced& operator = ( const Referenced& rhs ) { return *this; }

    LUNCHBOX_API virtual void notifyFree();

private:
    mutable a_int32_t _refCount;
    bool _hasBeenDeleted;

#ifdef LUNCHBOX_REFERENCED_DEBUG
    typedef PtrHash< const void*, std::string > HolderHash;
    mutable Lockable< HolderHash, SpinLock > _holders;
#endif
};
}

namespace boost
{
/** Allow creation of boost::intrusive_ptr from RefPtr or Referenced. */
inline void intrusive_ptr_add_ref( lunchbox::Referenced* referenced )
{
    referenced->ref();
}

/** Allow creation of boost::intrusive_ptr from RefPtr or Referenced. */
inline void intrusive_ptr_release( lunchbox::Referenced* referenced )
{
    referenced->unref();
}
}

#endif //LUNCHBOX_REFERENCED_H
