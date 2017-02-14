
/* Copyright (c) 2009-2017, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_LOCKABLE_H
#define LUNCHBOX_LOCKABLE_H

#include <lunchbox/log.h> // used inline

#include <boost/noncopyable.hpp>
#include <iostream>
#include <mutex>

namespace lunchbox
{
/**
 * A convenience structure to hold data together with a lock for access.
 *
 * Locking the data still has to be done manually, e.g, using a ScopedMutex.
 */
template <class D, class L = std::mutex>
class Lockable : public boost::noncopyable
{
public:
    /** Construct a new lockable data structure. @version 1.0 */
    Lockable() {}
    /** Construct and initialize a new data structure. @version 1.0 */
    explicit Lockable(const D& value)
        : data(value)
    {
    }

    /** Construct and initialize a new data structure. @version 1.3.2 */
    template <class P1>
    Lockable(const P1& p1)
        : data(p1)
    {
    }

    /** Access the held data. @version 1.0 */
    D* operator->() { return &data; }
    /** Access the held data. @version 1.0 */
    const D* operator->() const { return &data; }
    /** Access the held data. @version 1.1.5 */
    D& operator*() { return data; }
    /** Access the held data. @version 1.1.5 */
    const D& operator*() const { return data; }
    /** @return true if the data is equal to the rhs object. @version 1.0*/
    bool operator==(const D& rhs) const { return (data == rhs); }
    /** Assign another value to the data. @version 1.0 */
    Lockable& operator=(const D& rhs)
    {
        data = rhs;
        return *this;
    }

    D data;
    mutable L lock;
};

/** Print the data to the given output stream. */
template <class D, class L>
inline std::ostream& operator<<(std::ostream& os, const Lockable<D, L>& l)
{
    return os << disableFlush << "<" << l.lock.isSet() << " " << l.data << ">"
              << enableFlush;
}
}
#endif // LUNCHBOX_LOCKABLE_H
