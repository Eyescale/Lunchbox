
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#include <lunchbox/future.h> // base class
#include <boost/function/function0.hpp>

namespace lunchbox
{
/** A boolean future always returning true. Fully thread safe. @version 1.9.1 */
class FutureTrue : public FutureImpl< bool >
{
    bool wait() final { return true; }
    bool isReady() const final { return true; }
};

/**
 * A Future implementation using a boost::function for fulfilment.
 * Not thread safe.
 * @version 1.9.1
 */
template< class T > class FutureFunction : public FutureImpl< T >
{
public:
    typedef boost::function< T() > Func; //!< The fulfilling function

    FutureFunction( const Func& func ) : func_( func ) {}

private:
    virtual ~FutureFunction() { wait(); }

    virtual T wait() final
    {
        if( !func_.empty( ))
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
#endif //LUNCHBOX_FUTURE_H
