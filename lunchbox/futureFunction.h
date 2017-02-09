
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

#ifndef LUNCHBOX_FUTUREFUNCTION_H
#define LUNCHBOX_FUTUREFUNCTION_H

#include <boost/function/function0.hpp>
#include <lunchbox/future.h> // base class

namespace lunchbox
{
/**
 * A Future implementation using a boost::function for fulfilment.
 * Not thread safe.
 */
template <class T>
class FutureFunction : public FutureImpl<T>
{
public:
    typedef boost::function<T()> Func; //!< The fulfilling function

    explicit FutureFunction(const Func& func)
        : func_(func)
    {
    }

protected:
    virtual ~FutureFunction() { wait(0); }
    T wait(const uint32_t) final
    {
        if (!func_.empty())
        {
            result_ = func_();
            func_.clear();
        }
        return result_;
    }

    bool isReady() const final { return func_.empty(); }
    Func func_;
    T result_;
};
}
#endif // LUNCHBOX_FUTURE_H
