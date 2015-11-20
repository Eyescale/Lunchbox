
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Marwan Abdellah <marwan.abdellah@epfl.ch>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "thread.h"

#include "os.h"
#include "debug.h"
#include "lock.h"
#include "log.h"
#include "monitor.h"
#include "rng.h"
#include "scopedMutex.h"
#include "sleep.h"
#include "spinLock.h"

#include <boost/lexical_cast.hpp>
#include <errno.h>
#include <pthread.h>
#include <algorithm>
#include <map>
#include <set>

// Experimental Win32 thread pinning
#ifdef _WIN32
//#  define LB_WIN32_THREAD_AFFINITY
#  pragma message ("Thread affinity not supported on WIN32")
#endif

#ifndef _MSC_VER
#  include <signal.h>
#endif

#ifdef __linux__
#  include <sys/prctl.h>
#endif

#ifdef LUNCHBOX_USE_HWLOC
#  include <hwloc.h>
#endif

#include "detail/threadID.h"

namespace lunchbox
{
namespace
{
a_int32_t _threadIDs;

enum ThreadState //!< The current state of a thread.
{
    STATE_STOPPED,
    STATE_STARTING, // start() in progress
    STATE_RUNNING,
    STATE_STOPPING  // child no longer active, join() not yet called
};

#ifndef _MSC_VER
static Lockable< std::set< ThreadID >, SpinLock > _threads;
void _sigUserHandler( int, siginfo_t*, void* )
{
    LBERROR << ":" << backtrace << std::endl;
}
#endif

}

namespace detail
{
class Thread
{
public:
    Thread() : state( STATE_STOPPED ), index( ++_threadIDs ) {}

    lunchbox::ThreadID id;
    Monitor< ThreadState > state;
    int32_t index;
};
}

Thread::Thread()
    : _impl( new detail::Thread )
{
}

Thread::Thread( const Thread& )
    : _impl( new detail::Thread )
{
}

Thread::~Thread()
{
    delete _impl;
}

bool Thread::isStopped() const
{
    return ( _impl->state == STATE_STOPPED );
}

bool Thread::isRunning() const
{
    return ( _impl->state == STATE_RUNNING );
}

void* Thread::runChild( void* arg )
{
    Thread* thread = static_cast<Thread*>(arg);
    thread->_runChild();
    return 0; // not reached
}

void Thread::_runChild()
{
    setName( boost::lexical_cast< std::string >( _impl->index ));
    _impl->id._impl->pthread = pthread_self();
#ifndef _MSC_VER
    {
        ScopedFastWrite mutex( _threads );
        _threads->insert( _impl->id );
    }
    // install signal handler to dump thread state for debugging
    struct sigaction sa;
    ::sigfillset( &sa.sa_mask );
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = _sigUserHandler;
    ::sigaction( SIGUSR1, &sa, 0 );
#endif

    if( !init( ))
    {
        LBWARN << "Thread " << className( this ) << " failed to initialize"
               << std::endl;
        _impl->state = STATE_STOPPED;
        pthread_exit( 0 );
        LBUNREACHABLE;
    }

    _impl->state = STATE_RUNNING;
    LBDEBUG << "Thread #" << _impl->index << " type " << className( *this )
           << " successfully initialized" << std::endl;

    run();
    LBVERB << "Thread " << className( this ) << " finished" << std::endl;
#ifndef _MSC_VER
    {
        ScopedFastWrite mutex( _threads );
        _threads->erase( _impl->id );
    }
#endif
    this->exit();

    LBUNREACHABLE;
}

bool Thread::start()
{
    if( _impl->state != STATE_STOPPED )
        return false;

    _impl->state = STATE_STARTING;

    pthread_attr_t attributes;
    pthread_attr_init( &attributes );
    pthread_attr_setscope( &attributes, PTHREAD_SCOPE_SYSTEM );

    int nTries = 10;
    while( nTries-- )
    {
        const int error = pthread_create( &_impl->id._impl->pthread,
                                          &attributes, runChild, this );
        if( error == 0 ) // succeeded
        {
            LBVERB << "Created pthread " << this << std::endl;
            break;
        }
        if( error != EAGAIN || nTries == 0 )
        {
            LBWARN << "Could not create thread: " << strerror( error )
                   << std::endl;
            return false;
        }
        sleep( 1 ); // Give EAGAIN some time to recover
    }

    // avoid memleak, we don't use pthread_join
    pthread_detach( _impl->id._impl->pthread );
    _impl->state.waitNE( STATE_STARTING );
    return (_impl->state != STATE_STOPPED);
}

void Thread::exit()
{
    LBASSERTINFO( isCurrent(), "Thread::exit not called from child thread" );
    LBVERB << "Exiting thread " << className( this ) << std::endl;
    Log::instance().forceFlush();
    Log::instance().exit();

    _impl->state = STATE_STOPPING;
    pthread_exit( 0 );
    LBUNREACHABLE;
}

void Thread::cancel()
{
    LBASSERTINFO( !isCurrent(), "Thread::cancel called from child thread" );

    LBVERB << "Canceling thread " << className( this ) << std::endl;
    _impl->state = STATE_STOPPING;

    const int error = pthread_cancel( _impl->id._impl->pthread );
    if( error !=  0 )
        LBWARN << "Could not cancel thread: " << strerror( error ) << std::endl;
}

bool Thread::join()
{
    if( _impl->state == STATE_STOPPED )
        return false;
    if( isCurrent( )) // can't join self
        return false;

    _impl->state.waitNE( STATE_RUNNING );
    _impl->state = STATE_STOPPED;

    LBVERB << "Joined thread " << className( this ) << std::endl;
    return true;
}

bool Thread::isCurrent() const
{
    return pthread_equal( pthread_self(), _impl->id._impl->pthread );
}

ThreadID Thread::getSelfThreadID()
{
    ThreadID threadID;
    threadID._impl->pthread = pthread_self();
    return threadID;
}

void Thread::yield()
{
#ifdef _MSC_VER
    ::Sleep( 0 ); // sleeps thread
    // or ::SwitchToThread() ? // switches to another waiting thread, if exists
#elif defined (__APPLE__)
    ::pthread_yield_np();
#else
    ::sched_yield();
#endif
}

#ifdef _MSC_VER
#  ifndef MS_VC_EXCEPTION
#    define MS_VC_EXCEPTION 0x406D1388
#  endif

#  pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#  pragma pack(pop)
static void _setVCName( const char* name )
{
    ::Sleep(10);

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR),
                        (ULONG_PTR*)&info );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

void Thread::setName( const std::string& name )
{
    Log::instance().setThreadName( name );

#ifdef _MSC_VER
#  ifndef NDEBUG
    _setVCName( name.c_str( ));
#  endif
#elif __MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    pthread_setname_np( name.c_str( ));
#elif defined(__linux__)
    prctl( PR_SET_NAME, name.c_str(), 0, 0, 0 );
#else
    // Not implemented
    LBVERB << "Thread::setName() not implemented" << std::endl;
#endif
}

#ifdef LUNCHBOX_USE_HWLOC
static hwloc_bitmap_t _getCpuSet( const int32_t affinity,
                                  hwloc_topology_t topology )
{
    hwloc_bitmap_t cpuSet = hwloc_bitmap_alloc(); // HWloc CPU set
    hwloc_bitmap_zero( cpuSet ); // Initialize to zeros

    if( affinity >= Thread::CORE )
    {
        const int32_t coreIndex = affinity - Thread::CORE;
        if( hwloc_get_obj_by_type( topology, HWLOC_OBJ_CORE, coreIndex ) == 0 )
        {
            LBWARN << "Core " << coreIndex << " does not exist in the topology"
                   << std::endl;
            return cpuSet;
        }

        // Getting the core object #coreIndex
        const hwloc_obj_t coreObj = hwloc_get_obj_by_type( topology,
                                                           HWLOC_OBJ_CORE,
                                                           coreIndex );
        // Get the CPU set associated with the specified core
        cpuSet = coreObj->allowed_cpuset;
        return cpuSet;
    }

    if( affinity == Thread::NONE )
        return cpuSet;

    // Sets the affinity to a specific CPU or "socket"
    LBASSERT( affinity >= Thread::SOCKET && affinity < Thread::SOCKET_MAX );
    const int32_t socketIndex = affinity - Thread::SOCKET;

    if( hwloc_get_obj_by_type( topology, HWLOC_OBJ_SOCKET, socketIndex ) == 0 )
    {
        LBWARN << "Socket " << socketIndex << " does not exist in the topology"
               << std::endl;
        return cpuSet;
    }

    // Getting the CPU object #cpuIndex (subtree node)
    const hwloc_obj_t socketObj = hwloc_get_obj_by_type( topology,
                                                         HWLOC_OBJ_SOCKET,
                                                         socketIndex );
    // Get the CPU set associated with the specified socket
    hwloc_bitmap_copy( cpuSet, socketObj->allowed_cpuset );
    return cpuSet;
}
#endif

void Thread::setAffinity( const int32_t affinity )
{
    if( affinity == Thread::NONE )
        return;

#ifdef LUNCHBOX_USE_HWLOC
    hwloc_topology_t topology;
    hwloc_topology_init( &topology ); // Allocate & initialize the topology
    hwloc_topology_load( topology );  // Perform HW topology detection
    const hwloc_bitmap_t cpuSet = _getCpuSet( affinity, topology );
    const int result = hwloc_set_cpubind( topology, cpuSet,
                                          HWLOC_CPUBIND_THREAD );
    char* cpuSetString;
    hwloc_bitmap_asprintf( &cpuSetString, cpuSet );

    if( result == 0 )
    {
        LBVERB << "Bound to cpu set "  << cpuSetString << std::endl;
    }
    else
    {
        LBWARN << "Error binding to cpu set " << cpuSetString << std::endl;
    }
    ::free( cpuSetString );
    hwloc_bitmap_free( cpuSet );
    hwloc_topology_destroy( topology );

#else
    LBWARN << "Thread::setAffinity not implemented, hwloc library missing"
           << std::endl;
#endif
}

void Thread::_dumpAll()
{
#ifndef _MSC_VER
    ScopedFastRead mutex( _threads );
    for( const ThreadID& id : _threads.data )
        pthread_kill( id._impl->pthread, SIGUSR1 );
#endif
}

std::ostream& operator << ( std::ostream& os, const Thread::Affinity affinity )
{
    if( affinity == Thread::NONE )
        return os << "No affinity";
    if( affinity >= Thread::CORE )
        return os << "Core " << affinity - Thread::CORE;

    LBASSERT( affinity >= Thread::SOCKET && affinity < Thread::SOCKET_MAX );
    return os << "Socket " <<  affinity - Thread::SOCKET;
}

#if 0
std::ostream& operator << ( std::ostream& os, const Thread* thread )
{
    os << "Thread " << thread->_impl->id << " state "
       << ( thread->_impl->state == Thread::STATE_STOPPED  ? "stopped"  :
            thread->_impl->state == Thread::STATE_STARTING ? "starting" :
            thread->_impl->state == Thread::STATE_RUNNING  ? "running"  :
            thread->_impl->state == Thread::STATE_STOPPING ? "stopping" :
            "unknown" );

#ifdef PTW32_VERSION
    os << " called from " << pthread_self().p;
#else
    os << " called from " << pthread_self();
#endif

    return os;
}
#endif
}
