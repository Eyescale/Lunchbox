/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_MONITOR_H
#define LUNCHBOX_MONITOR_H

#include <lunchbox/scopedMutex.h> // used inline
#include <lunchbox/types.h>

#include <boost/bind.hpp>
#include <condition_variable>
#include <errno.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <string.h>
#include <typeinfo>

namespace lunchbox
{
/**
 * A monitor primitive.
 *
 * A monitor has a value, which can be monitored to reach a certain state. The
 * caller is blocked until the condition is fulfilled. The concept is similar to
 * a pthread condition, with more usage convenience.
 *
 * Example: @include tests/monitor.cpp
 */
template <class T>
class Monitor
{
    typedef void (Monitor<T>::*bool_t)() const;
    void bool_true() const {}
public:
    /** Construct a new monitor with a default value of 0. @version 1.0 */
    Monitor()
        : _value(T(0))
    {
    }

    /** Construct a new monitor with a given default value. @version 1.0 */
    explicit Monitor(const T& value)
        : _value(value)
    {
    }

    /** Ctor initializing with the given monitor value. @version 1.1.5 */
    Monitor(const Monitor<T>& from)
        : _value(from._value)
    {
    }

    /** Destructs the monitor. @version 1.0 */
    ~Monitor() {}
    /** @name Changing the monitored value. */
    //@{
    /** Increment the monitored value, prefix only. @version 1.0 */
    Monitor& operator++()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        ++_value;
        _condition.notify_all();
        return *this;
    }

    /** Decrement the monitored value, prefix only. @version 1.0 */
    Monitor& operator--()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        --_value;
        _condition.notify_all();
        return *this;
    }

    /** Assign a new value. @version 1.0 */
    Monitor& operator=(const T& value)
    {
        set(value);
        return *this;
    }

    /** Assign a new value. @version 1.1.5 */
    const Monitor& operator=(const Monitor<T>& from)
    {
        set(from._value);
        return *this;
    }

    /** Perform an or operation on the value. @version 1.0 */
    Monitor& operator|=(const T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _value |= value;
        _condition.notify_all();
        return *this;
    }

    /** Perform an and operation on the value. @version 1.7 */
    Monitor& operator&=(const T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _value &= value;
        _condition.notify_all();
        return *this;
    }

    /** Set a new value. @return the old value @version 1.0 */
    T set(const T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        const T old = _value;
        _value = value;
        _condition.notify_all();
        return old;
    }
    //@}

    /** @name Monitor the value. */
    //@{
    /**
     * Block until the monitor has the given value.
     * @return the value when reaching the condition.
     * @version 1.0
     */
    const T waitEQ(const T& value) const
    {
        return _wait([&] { return _value == value; });
    }

    /**
     * Block until the monitor has not the given value.
     * @return the value when reaching the condition.
     * @version 1.0
     */
    const T waitNE(const T& value) const
    {
        return _wait([&] { return _value != value; });
    }

    /**
     * Block until the monitor has none of the given values.
     * @return the value when reaching the condition.
     * @version 1.0
     */
    const T waitNE(const T& v1, const T& v2) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [&] { return _value != v1 && _value != v2; });
        return _value;
    }

    /**
     * Block until the monitor has a value greater or equal to the given value.
     * @return the value when reaching the condition.
     * @version 1.0
     */
    const T waitGE(const T& value) const
    {
        return _wait([&] { return _value >= value; });
    }

    /**
     * Block until the monitor has a value less or equal to the given value.
     * @return the value when reaching the condition.
     * @version 1.0
     */
    const T waitLE(const T& value) const
    {
        return _wait([&] { return _value <= value; });
    }

    /**
     * Block until the monitor has a value greater than the given value.
     * @return the value when reaching the condition.
     * @version 1.10
     */
    const T waitGT(const T& value) const
    {
        return _wait([&] { return _value > value; });
    }

    /**
     * Block until the monitor has a value less than the given value.
     * @return the value when reaching the condition.
     * @version 1.10
     */
    const T waitLT(const T& value) const
    {
        return _wait([&] { return _value < value; });
    }

    /** @name Monitor the value with a timeout. */
    //@{
    /**
     * Block until the monitor has the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for the value.
     * @return true on success, false on timeout.
     * @version 1.1
     */
    bool timedWaitEQ(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value == value; }, timeout);
    }

    /**
     * Block until the monitor has not the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for not the value.
     * @return true on success, false on timeout.
     * @version 1.10
     */
    bool timedWaitNE(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value != value; }, timeout);
    }

    /**
     * Block until the monitor has a value greater or equal to the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for the value.
     * @return true on success, false on timeout.
     * @version 1.1
     */
    bool timedWaitGE(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value >= value; }, timeout);
    }

    /**
     * Block until the monitor has a value less or equal to the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for the value.
     * @return true on success, false on timeout.
     * @version 1.10
     */
    bool timedWaitLE(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value <= value; }, timeout);
    }

    /**
     * Block until the monitor has a value greater than the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for the value.
     * @return true on success, false on timeout.
     * @version 1.10
     */
    bool timedWaitGT(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value > value; }, timeout);
    }

    /**
     * Block until the monitor has a value less than the given value.
     * @param value the exact value to monitor.
     * @param timeout the timeout in milliseconds to wait for the value.
     * @return true on success, false on timeout.
     * @version 1.10
     */
    bool timedWaitLT(const T& value, const uint32_t timeout) const
    {
        return _timedWait([&] { return _value < value; }, timeout);
    }

    //@}

    /** @name Comparison Operators. @version 1.0 */
    //@{
    bool operator==(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value == value;
    }
    bool operator!=(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value != value;
    }
    bool operator<(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value < value;
    }
    bool operator>(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value > value;
    }
    bool operator<=(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value <= value;
    }
    bool operator>=(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value >= value;
    }

    bool operator==(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value == rhs._value;
    }
    bool operator!=(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value != rhs._value;
    }
    bool operator<(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value < rhs._value;
    }
    bool operator>(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value > rhs._value;
    }
    bool operator<=(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value <= rhs._value;
    }
    bool operator>=(const Monitor<T>& rhs) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value >= rhs._value;
    }
    /** @return a bool conversion of the result. @version 1.9.1 */
    operator bool_t()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value ? &Monitor<T>::bool_true : 0;
    }
    //@}

    /** @name Data Access. */
    //@{
    /** @return the current value. @version 1.0 */
    const T& operator->() const { return _value; }
    /** @return the current value. @version 1.0 */
    const T& get() const { return _value; }
    /** @return the current plus the given value. @version 1.0 */
    T operator+(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _value + value;
    }

    /** @return the current or'ed with the given value. @version 1.0 */
    T operator|(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return static_cast<T>(_value | value);
    }

    /** @return the current and the given value. @version 1.0 */
    T operator&(const T& value) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return static_cast<T>(_value & value);
    }
    //@}

private:
    T _value;
    mutable std::mutex _mutex;
    mutable std::condition_variable _condition;

    template <typename F>
    const T _wait(const F& predicate) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, predicate);
        return _value;
    }

    template <typename F>
    bool _timedWait(const F& predicate, const uint32_t timeout) const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _condition.wait_for(lock, std::chrono::milliseconds(timeout),
                                   predicate);
    }
};

typedef Monitor<bool> Monitorb;     //!< A boolean monitor variable
typedef Monitor<uint32_t> Monitoru; //!< An unsigned 32bit integer monitor

/** Print the monitor to the given output stream. @version 1.0 */
template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Monitor<T>& monitor)
{
    os << "Monitor< " << monitor.get() << " >";
    return os;
}

template <>
inline Monitor<bool>& Monitor<bool>::operator++()
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(!_value);
    _value = !_value;
    _condition.notify_all();
    return *this;
}

template <>
inline Monitor<bool>& Monitor<bool>::operator--()
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(!_value);
    _value = !_value;
    _condition.notify_all();
    return *this;
}

template <>
inline Monitor<bool>& Monitor<bool>::operator|=(const bool& value)
{
    if (value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _value = value;
        _condition.notify_all();
    }
    return *this;
}
}

#include <servus/uint128_t.h>
namespace lunchbox
{
template <>
inline Monitor<servus::uint128_t>::Monitor()
{
}
}

#endif // LUNCHBOX_MONITOR_H
