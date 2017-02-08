
/* Copyright (c) 2013-2014, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_READYFUTURE_H
#define LUNCHBOX_READYFUTURE_H

#include <lunchbox/future.h> // base class

namespace lunchbox
{
/** A boolean future with a known value. Fully thread safe. */
template <bool value>
class FutureBool : public FutureImpl<bool>
{
protected:
    bool wait(const uint32_t) final { return value; }
    bool isReady() const final { return true; }
};

/** @return a boolean future being true. */
inline Future<bool> makeTrueFuture()
{
    return Future<bool>(new FutureBool<true>);
}

/** @return a boolean future being false. */
inline Future<bool> makeFalseFuture()
{
    return Future<bool>(new FutureBool<false>);
}
}
#endif // LUNCHBOX_READYFUTURE_H
