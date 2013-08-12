
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

#ifndef LUNCHBOX_FUTURE_H
#define LUNCHBOX_FUTURE_H

#include <lunchbox/referenced.h>
#include <lunchbox/refPtr.h>
#include <boost/function/function0.hpp>

namespace lunchbox
{

/** @internal subclass to implement the wait method fulfilling the future. */
template< class T > class FutureImpl : public Referenced
{
public:
    /** Destruct the future. @version 1.9.1 */
    virtual ~FutureImpl(){}

    /**
     * Wait for the promise to be fullfilled.
     *
     * May be called multiple times.
     *
     * @version 1.9.1 */
    virtual T wait() = 0;
};

/**
 * A Future implementation using a boost::function for fulfilment.
 * @version 1.9.1
 */
template< class T > class FutureFunction : public FutureImpl< T >
{
public:
    typedef boost::function< T() > Func;

    FutureFunction( const Func& func ) : func_( func ) {}
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

private:
    Func func_;
    T result_;
};

/** A future represents a asynchronous operation. Do not subclass. */
template< class T > class Future
{
    typedef void (Future< T >::*bool_t)() const;
    void bool_true() const {}

public:
    typedef RefPtr< FutureImpl< T > > Impl; //!< The wait implementation

    /** Construct a new future. @version 1.9.1 */
    Future( Impl impl ) : impl_( impl ){}

    /** Destruct the future. @version 1.9.1 */
     ~Future(){}

    /** Wait for the promise to be fullfilled. @version 1.9.1 */
    T wait() { return impl_->wait(); }

    /** @name Blocking comparison operators. */
    //@{
    /** @return a bool conversion of the result. @version 1.9.1 */
    operator bool_t() { return wait() ? &Future< T >::bool_true : 0; }

    /** @return true if the result does convert to false. @version 1.9.1 */
    bool operator ! () { return !wait(); }

    /** @return true if the result is equal to the given value. @version 1.9.1*/
    bool operator == ( const T& rhs ) { return wait() == rhs; }

    /** @return true if the result is not equal to the rhs. @version 1.9.1*/
    bool operator != ( const T& rhs ) { return wait() != rhs; }

    /** @return true if the result is smaller than the rhs. @version 1.9.1 */
    bool operator < ( const T& rhs ) { return wait() < rhs; }

    /** @return true if the result is bigger than the rhs. @version 1.9.1 */
    bool operator > ( const uint128_t& rhs ) { return wait() > rhs; }

    /** @return true if the result is smaller or equal. @version 1.9.1 */
    bool operator <= ( const uint128_t& rhs ) { return wait() <= rhs; }

    /** @return true if the result is bigger or equal. @version 1.9.1 */
    bool operator >= ( const uint128_t& rhs ) { return wait() >= rhs; }
    //@}

private:
    Impl impl_;
};

typedef Future< bool > Futureb; //!< A future with a boolean return value

}
#endif //LUNCHBOX_FUTURE_H
