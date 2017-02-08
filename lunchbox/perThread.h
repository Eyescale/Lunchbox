
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_PERTHREAD_H
#define LUNCHBOX_PERTHREAD_H

#include <lunchbox/compiler.h> // deprecated macro
#include <lunchbox/debug.h>    // LBASSERTINFO
#include <lunchbox/tls.h>      // member

namespace lunchbox
{
namespace detail
{
class PerThread;
}

/** Default PerThread destructor deleting the object. @version 1.1.2 */
template <class T>
void perThreadDelete(T* object)
{
    delete object;
}

/** Empty PerThread destructor. @version 1.1.2 */
template <class T>
void perThreadNoDelete(T*)
{
}

/**
 * Implements thread-specific storage for C++ objects.
 *
 * The default destructor function deletes the object on thread exit.
 *
 * @param T the type of data to store in thread-local storage
 * @param D the destructor callback function.
 * @deprecated Use boost::thread_specific_ptr
 *
 * Example: @include tests/perThread.cpp
 */
template <class T, void (*D)(T*) = &perThreadDelete<T> >
class PerThread
{
public:
    /** Construct a new per-thread variable. @version 1.0 */
    PerThread();
    /** Destruct the per-thread variable. @version 1.0 */
    ~PerThread();

    /** Assign an object to the thread-local storage. @version 1.0 */
    PerThread<T, D>& operator=(const T* data);
    /** Assign an object from another thread-local storage. @version 1.0 */
    PerThread<T, D>& operator=(const PerThread<T, D>& rhs);

    /** @return the held object pointer. @version 1.0 */
    T* get();
    /** @return the held object pointer. @version 1.0 */
    const T* get() const;
    /** Access the thread-local object. @version 1.0 */
    T* operator->();
    /** Access the thread-local object. @version 1.0 */
    const T* operator->() const;

    /** @return the held object reference. @version 1.0 */
    T& operator*()
    {
        LBASSERTINFO(get(), className(this));
        return *get();
    }
    /** @return the held object reference. @version 1.0 */
    const T& operator*() const
    {
        LBASSERTINFO(get(), className(this));
        return *get();
    }

    /**
     * @return true if the thread-local variables hold the same object.
     * @version 1.0
     */
    bool operator==(const PerThread& rhs) const { return (get() == rhs.get()); }
    /**
     * @return true if the thread-local variable holds the same object.
     * @version 1.0
     */
    bool operator==(const T* rhs) const { return (get() == rhs); }
    /**
     * @return true if the thread-local variable holds another object.
     * @version 1.0
     */
    bool operator!=(const T* rhs) const { return (get() != rhs); }
    /**
     * @return true if the thread-local storage holds a 0 pointer.
     * @version 1.0
     */
    bool operator!() const;

    /**
     * @return true if the thread-local storage holds a non-0 pointer.
     * @version 1.0
     */
    bool isValid() const;

private:
    TLS tls_;
    PerThread(const PerThread&) = delete;
    PerThread(PerThread&&) = delete;
    PerThread& operator=(const PerThread&&) = delete;
};
}

#include "perThread.ipp" // template implementation

#endif // LUNCHBOX_PERTHREAD_H
