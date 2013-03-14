
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2012, Marwan Abdellah <marwan.abdellah@epfl.ch>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_THREAD_H
#define LUNCHBOX_THREAD_H

#include <lunchbox/api.h>         // LUNCHBOX_API definition
#include <lunchbox/debug.h>       // debug macros in thread-safety checks
#include <lunchbox/nonCopyable.h> // base class
#include <lunchbox/threadID.h>    // member
#include <lunchbox/types.h>

#include <ostream>

namespace lunchbox
{
namespace detail { class Thread; }

/** An utility class to execute code in a separate execution thread. */
class Thread : public NonCopyable
{
public:
    /** Enumeration values for thread affinity. */
    enum Affinity
    {
        NONE = 0, //!< Don't set any affinity
        CORE = 1, //!< Bind to a specific CPU core
        SOCKET = -65536, //!< Bind to all cores of a specific socket (CPU)
        SOCKET_MAX = -1024 //!< Highest bindable CPU
    };

    /** Construct a new thread. @version 1.0 */
    LUNCHBOX_API Thread();

    /** Copy constructor. @version 1.1.2 */
    LUNCHBOX_API Thread( const Thread& from );

    /** Destruct the thread. @version 1.0 */
    LUNCHBOX_API virtual ~Thread();

    /**
     * Start the thread.
     *
     * @return <code>true</code> if the thread was launched and initialized
     *         successfully, <code>false</code> otherwise.
     * @sa init(), run()
     * @version 1.0
     */
    LUNCHBOX_API virtual bool start();

    /**
     * The init function for the child thread.
     *
     * The parent thread will not be unlocked before this function has been
     * executed. If the thread initialization fails, that is, this method
     * does return false, the thread will be stopped and the start() method
     * will return false.
     *
     * @return the success value of the thread initialization.
     * @version 1.0
     */
    virtual bool init(){ return true; }

    /**
     * The entry function for the child thread.
     *
     * This method should contain the main execution routine for the thread
     * and is called after a successful init().
     *
     * @version 1.0
     */
    virtual void run() = 0;

    /**
     * Exit the child thread immediately.
     *
     * This function does not return. It is only to be called from the child
     * thread.
     *
     * @version 1.0
     */
    LUNCHBOX_API virtual void exit();

    /**
     * Cancel (stop) the child thread.
     *
     * This function is not to be called from the child thread.
     * @version 1.0
     */
    LUNCHBOX_API void cancel();

    /**
     * Wait for the exit of the child thread.
     *
     * @return true if the thread was joined, false otherwise.
     * @version 1.0
     */
    LUNCHBOX_API bool join();

    /**
     * Return if the thread is stopped.
     *
     * Note that the thread may be neither running nor stopped if it is
     * currently starting or stopping.
     *
     * @return true if the thread is stopped, false if not.
     * @version 1.0
     */
    LUNCHBOX_API bool isStopped() const;

    /**
     * Return if the thread is running.
     *
     * Note that the thread may be neither running nor stopped if it is
     * currently starting or stopping.
     *
     * @return true if the thread is running, false if not.
     * @version 1.0
     */
    LUNCHBOX_API bool isRunning() const;

    /**
     * @return true if the calling thread is the same thread as this
     *         thread, false otherwise.
     * @version 1.0
     */
    LUNCHBOX_API bool isCurrent() const;

    /** @return a unique identifier for the calling thread. @version 1.0 */
    LUNCHBOX_API static ThreadID getSelfThreadID();

    /** @internal */
    LUNCHBOX_API static void yield();

    /** @internal */
    static void pinCurrentThread();

    /** @internal */
    LUNCHBOX_API static void setName( const std::string& name );

    /** @internal
     * Set the affinity of the calling thread.
     *
     * If given a value greater or equal than CORE, this method binds the
     * calling thread to core affinity - CORE. If set to a value greater
     * than CPU and smaller than 0, this method binds the calling thread to
     * all cores of the given processor (affinity - CPU).
     *
     * @param affinity the affinity value (see above).
     */
    LUNCHBOX_API static void setAffinity( const int32_t affinity );

private:
    detail::Thread* const _impl;

    static void* runChild( void* arg );
    void        _runChild();

    void _installCleanupHandler();

    static void _notifyStarted();
    static void _notifyStopping();
    friend void _notifyStopping( void* ); //!< @internal
};

/** Output the affinity setting in human-readable form. @version 1.7.1 */
LUNCHBOX_API std::ostream& operator << ( std::ostream&, const Thread::Affinity );

// thread-safety checks
// These checks are for development purposes, to check that certain objects are
// properly used within the framework. Leaving them enabled during application
// development may cause false positives, e.g., when threadsafety is ensured
// outside of the objects by the application.

#ifndef NDEBUG
#  define LB_CHECK_THREADSAFETY
#endif

/** Declare a thread id variable to be used for thread-safety checks. */
#define LB_TS_VAR( NAME )                       \
    public:                                     \
    struct NAME ## Struct                       \
    {                                           \
        NAME ## Struct ()                       \
            : extMutex( false )                 \
        {}                                      \
        mutable lunchbox::ThreadID id;          \
        mutable std::string name;               \
        bool extMutex;                          \
        mutable lunchbox::ThreadID inRegion;    \
    } NAME;                                     \
private:

#ifdef LB_CHECK_THREADSAFETY
#  define LB_TS_RESET( NAME ) NAME.id = lunchbox::ThreadID();

#  define LB_TS_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id == lunchbox::ThreadID( ))                           \
        {                                                               \
            NAME.id = lunchbox::Thread::getSelfThreadID();              \
            NAME.name = lunchbox::Log::instance().getThreadName();      \
            LBVERB << "Functions for " << #NAME                         \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !NAME.extMutex && NAME.id != lunchbox::Thread::getSelfThreadID( )) \
        {                                                               \
            LBERROR << "Threadsafety check for " << #NAME               \
                    << " failed on object of type "                     \
                    << lunchbox::className( this ) << ", thread "       \
                    << lunchbox::Thread::getSelfThreadID() << " ("      \
                    << lunchbox::Log::instance().getThreadName() << ") != " \
                    << NAME.id << " (" << NAME.name << ")" << std::endl; \
            LBABORT( "Non-threadsave code called from two threads" );   \
        }                                                               \
    }

#  define LB_TS_NOT_THREAD( NAME )                                      \
    {                                                                   \
        if( !NAME.extMutex && NAME.id != lunchbox::ThreadID( ))         \
        {                                                               \
            if( NAME.id == lunchbox::Thread::getSelfThreadID( ))        \
            {                                                           \
                LBERROR << "Threadsafety check for not " << #NAME       \
                        << " failed on object of type "                 \
                        << lunchbox::className( this ) << std::endl;    \
                LBABORT( "Code called from wrong thread" );             \
            }                                                           \
        }                                                               \
    }

/** @cond IGNORE */
template< typename T > class ScopedThreadCheck : public NonCopyable
{
public:
    explicit ScopedThreadCheck( const T& data )
    : _data( data )
        {
            LBASSERTINFO( data.inRegion == lunchbox::ThreadID() ||
                          data.inRegion == lunchbox::Thread::getSelfThreadID(),
                          "Another thread already in critical region" );
            data.inRegion = lunchbox::Thread::getSelfThreadID();
        }

    ~ScopedThreadCheck()
        {
            LBASSERTINFO( _data.inRegion == lunchbox::ThreadID() ||
                          _data.inRegion == lunchbox::Thread::getSelfThreadID(),
                          "Another thread entered critical region" );
            _data.inRegion = lunchbox::ThreadID();
        }
private:
    const T& _data;
};
/** @endcond */

# define LB_TS_SCOPED( NAME )                                           \
    lunchbox::ScopedThreadCheck< NAME ## Struct > scoped ## NAME ## Check(NAME);

#else
#  define LB_TS_RESET( NAME ) {}
#  define LB_TS_THREAD( NAME ) {}
#  define LB_TS_NOT_THREAD( NAME ) {}
#  define LB_TS_SCOPED( NAME ) {}
#endif

}
#endif //LUNCHBOX_THREAD_H
