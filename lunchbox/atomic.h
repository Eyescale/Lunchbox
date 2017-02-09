
//  Copyright (C) 2007, 2008 Tim Blechmann & Thomas Grill
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com>
   Modifications to use within lunchbox namespace and naming conventions.
   Original at http://tim.klingt.org/git?p=boost_lockfree.git;a=tree
*/

#ifndef LUNCHBOX_ATOMIC_H
#define LUNCHBOX_ATOMIC_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h> // GCC version
#include <lunchbox/types.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4985) // inconsistent decl of ceil
#include <intrin.h>
#include <math.h> // include math.h early to avoid warning later
#pragma warning(pop)
#pragma intrinsic(_ReadWriteBarrier)
#elif defined(__xlC__)
#include <builtins.h>
#include <iostream>
#endif

namespace lunchbox
{
/** Perform a full memory barrier. */
inline void memoryBarrier()
{
#ifdef __GNUC__
    __sync_synchronize();
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
#elif defined(__xlC__)
    __fence();
    __eieio();
    __fence();
#else
#error "no memory barrier implemented for this platform"
#endif
}

/** Perform a load-with-acquire memory barrier. */
inline void memoryBarrierAcquire()
{
#ifdef __xlC__
    __fence();
    __eieio();
#else
    memoryBarrier();
#endif
}

/** Perform a store-with-release memory barrier. */
inline void memoryBarrierRelease()
{
#ifdef __xlC__
    __isync();
    __fence();
#else
    memoryBarrier();
#endif
}

/**
 * A variable with atomic semantics and standalone atomic operations.
 *
 * Use the C++11 equivalent if you can.
 *
 * Atomic variables can be modified safely from multiple threads
 * concurrently. They are useful to implement lock-free algorithms.
 *
 * For implementation reasons, only signed atomic variables are supported, of
 * which int32_t and ssize_t are implemented and typedef'd as a_int32_t and
 * a_ssize_t.
 */
template <class T>
class Atomic
{
public:
    /** @return the old value, then add the given increment. */
    LUNCHBOX_API static T getAndAdd(T& value, const T increment);

    /** @return the old value, then substract the increment. */
    LUNCHBOX_API static T getAndSub(T& value, const T increment);

    /** @return the new value after adding the given increment. */
    static T addAndGet(T& value, const T increment);

    /** @return the new value after substracting the increment. */
    static T subAndGet(T& value, const T increment);

    /** @return the new value after incrementing the value. */
    LUNCHBOX_API static T incAndGet(T& value);

    /** @return the new value after decrementing the value. */
    LUNCHBOX_API static T decAndGet(T& value);

    /** Perform a compare-and-swap atomic operation. */
    LUNCHBOX_API static bool compareAndSwap(T* value, const T expected,
                                            const T newValue);

    /** Construct a new atomic variable with an initial value. @version 1.0 */
    explicit Atomic(const T v = 0);

    /** Construct a copy of an atomic variable. Not thread-safe! @version 1.0 */
    Atomic(const Atomic<T>& v);

    /** @return the current value @version 1.0 */
    operator T(void) const;

    /** Assign a new value @version 1.0 */
    void operator=(const T v);

    /** Assign a new value. Not thread-safe! @version 1.0 */
    void operator=(const Atomic<T>& v);

    /** Atomically add a value and return the new value. @version 1.0 */
    T operator+=(T v);

    /** Atomically substract a value and return the new value. @version 1.0 */
    T operator-=(T v);

    /** Atomically increment by one and return the new value. @version 1.0 */
    T operator++(void);

    /** Atomically decrement by one and return the new value. @version 1.0 */
    T operator--(void);

    /** Atomically increment by one and return the old value. @version 1.0 */
    T operator++(int);

    /** Atomically decrement by one and return the old value. @version 1.0 */
    T operator--(int);

    /** @return true if the variable has the given value. @version 1.1.2 */
    bool operator==(const Atomic<T>& rhs) const;

    /** @return true if the variable has not the given value. @version 1.1.2 */
    bool operator!=(const Atomic<T>& rhs) const;

    /**
     * Perform a compare-and-swap atomic operation.
     *
     * Atomically replaces the value and return true if the value matched the
     * expected.
     * @return true if the new value was set, false otherwise
     * @version 1.1.2
     */
    bool compareAndSwap(const T expected, const T newValue);

private:
// https://github.com/Eyescale/Lunchbox/issues/8
#if _MSC_VER < 1700
    mutable T _value;
#else
    LB_ALIGN8(mutable T _value);
#endif
};

// Implementation
#ifdef __GNUC__
template <class T>
T Atomic<T>::getAndAdd(T& value, const T increment)
{
    return __sync_fetch_and_add(&value, increment);
}

template <class T>
T Atomic<T>::getAndSub(T& value, const T increment)
{
    return __sync_fetch_and_sub(&value, increment);
}

template <class T>
T Atomic<T>::addAndGet(T& value, const T increment)
{
    return __sync_add_and_fetch(&value, increment);
}

template <class T>
T Atomic<T>::subAndGet(T& value, const T increment)
{
    return __sync_sub_and_fetch(&value, increment);
}

template <class T>
T Atomic<T>::incAndGet(T& value)
{
    return addAndGet(value, 1);
}

template <class T>
T Atomic<T>::decAndGet(T& value)
{
    return subAndGet(value, 1);
}

template <class T>
bool Atomic<T>::compareAndSwap(T* value, const T expected, const T newValue)
{
    return __sync_bool_compare_and_swap(value, expected, newValue);
}

#elif defined(_MSC_VER)

// see also atomic.cpp
template <class T>
T Atomic<T>::addAndGet(T& value, const T increment)
{
    return getAndAdd(value, increment) + increment;
}

template <class T>
T Atomic<T>::subAndGet(T& value, const T increment)
{
    return getAndSub(value, increment) - increment;
}

#else
#ifdef __xlC__
template <class T>
bool Atomic<T>::compareAndSwap(T* value, const T expected, const T newValue)
{
    return __compare_and_swap(value, const_cast<T*>(&expected), newValue);
}
#ifdef __64BIT__
template <>
inline bool Atomic<int64_t>::compareAndSwap(int64_t* value,
                                            const int64_t expected,
                                            const int64_t newValue)
{
    return __compare_and_swaplp(value, const_cast<int64_t*>(&expected),
                                newValue);
}
#endif
#else
#error No compare-and-swap implementated for this platform
#endif

template <class T>
T Atomic<T>::getAndAdd(T& value, const T increment)
{
    for (;;)
    {
        memoryBarrierAcquire();
        const T oldv = value;
        const T newv = oldv + increment;
        if (!compareAndSwap(&value, oldv, newv))
            continue;

        memoryBarrierRelease();
        return oldv;
    }
}

template <class T>
T Atomic<T>::getAndSub(T& value, const T increment)
{
    for (;;)
    {
        memoryBarrierAcquire();
        const T oldv = value;
        const T newv = oldv - increment;
        if (!compareAndSwap(&value, oldv, newv))
            continue;

        memoryBarrierRelease();
        return oldv;
    }
}

template <class T>
T Atomic<T>::addAndGet(T& value, const T increment)
{
    for (;;)
    {
        memoryBarrierAcquire();
        const T oldv = value;
        const T newv = oldv + increment;
        if (!Atomic<T>::compareAndSwap(&value, oldv, newv))
            continue;

        memoryBarrierRelease();
        return newv;
    }
}

template <class T>
T Atomic<T>::subAndGet(T& value, const T increment)
{
    for (;;)
    {
        memoryBarrierAcquire();
        const T oldv = value;
        const T newv = oldv - increment;
        if (!Atomic<T>::compareAndSwap(&value, oldv, newv))
            continue;

        memoryBarrierRelease();
        return newv;
    }
}

template <class T>
T Atomic<T>::incAndGet(T& value)
{
    return addAndGet(value, 1);
}

template <class T>
T Atomic<T>::decAndGet(T& value)
{
    return subAndGet(value, 1);
}
#endif

template <class T>
Atomic<T>::Atomic(const T v)
    : _value(v)
{
}

template <class T>
Atomic<T>::Atomic(const Atomic<T>& v)
    : _value(v._value)
{
}

template <class T>
Atomic<T>::operator T(void) const
{
    memoryBarrierAcquire();
    return _value;
}

template <class T>
void Atomic<T>::operator=(const T v)
{
    _value = v;
    memoryBarrier();
}

template <class T>
void Atomic<T>::operator=(const Atomic<T>& v)
{
    _value = v._value;
    memoryBarrier();
}

template <class T>
T Atomic<T>::operator+=(T v)
{
    return addAndGet(_value, v);
}

template <class T>
T Atomic<T>::operator-=(T v)
{
    return subAndGet(_value, v);
}

template <class T>
T Atomic<T>::operator++(void)
{
    return incAndGet(_value);
}

template <class T>
T Atomic<T>::operator--(void)
{
    return decAndGet(_value);
}

template <class T>
T Atomic<T>::operator++(int)
{
    return getAndAdd(_value, 1);
}

template <class T>
T Atomic<T>::operator--(int)
{
    return getAndSub(_value, 1);
}

template <class T>
bool Atomic<T>::operator==(const Atomic<T>& rhs) const
{
    memoryBarrier();
    return _value == rhs._value;
}

template <class T>
bool Atomic<T>::operator!=(const Atomic<T>& rhs) const
{
    memoryBarrier();
    return _value != rhs._value;
}

template <class T>
bool Atomic<T>::compareAndSwap(const T expected, const T newValue)
{
    return compareAndSwap(&_value, expected, newValue);
}
}
#endif // LUNCHBOX_ATOMIC_H
