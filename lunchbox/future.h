
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

#ifndef LUNCHBOX_FUTURE_H
#define LUNCHBOX_FUTURE_H

#include <lunchbox/refPtr.h>      // used inline
#include <lunchbox/referenced.h>  // base class

#include <stdexcept>

namespace lunchbox
{

class FutureTimeout : public std::runtime_error
{
public:
    FutureTimeout() : std::runtime_error("") {}
};

/** Base class to implement the wait method fulfilling the future. */
template< class T >
class FutureImpl : public Referenced, public boost::noncopyable
{
public:
    /** Destruct the future. */
    virtual ~FutureImpl(){}

    /**
     * Wait for the promise to be fullfilled.
     *
     * May be called multiple times.
     * @param timeout optional timeout in milliseconds. If the future is
     *                not ready when the timer goes off a timeout exception
     *                is thrown.
     */
    virtual T wait( const uint32_t timeout = LB_TIMEOUT_INDEFINITE ) = 0;

    /**
     * @return true if the future has been fulfilled, false if it is pending.
     */
    virtual bool isReady() const = 0;
};

/** A future represents a asynchronous operation. Do not subclass. */
template< class T > class Future
{
private:
    typedef void (Future< T >::*bool_t)() const;
    void bool_true() const {}

public:
    typedef RefPtr< FutureImpl< T > > Impl; //!< The wait implementation

    /** Construct a new future. */
    explicit Future( Impl impl ) : impl_( impl ){}

    /** Destruct the future. */
     ~Future(){}

    /**
     * Wait for the promise to be fullfilled.
     *
     * @param timeout_ optional timeout in milliseconds. If the future is
     *                 not ready when the timer goes off a timeout exception
     *                 is thrown.
     */
    T wait( const uint32_t timeout_ = LB_TIMEOUT_INDEFINITE )
    {
        return impl_->wait( timeout_ );
    }

    /**
     * @return true if the future has been fulfilled, false if it is pending.
     */
    bool isReady() const { return impl_->isReady(); }

    /** @name Blocking comparison operators. */
    //@{
    /** @return a bool conversion of the result. */
    operator bool_t() { return wait() ? &Future< T >::bool_true : 0; }

    /** @return true if the result does convert to false. */
    bool operator ! () { return !wait(); }

    /** @return true if the result is equal to the given value. */
    bool operator == ( const T& rhs ) { return wait() == rhs; }

    /** @return true if the result is not equal to the rhs. */
    bool operator != ( const T& rhs ) { return wait() != rhs; }

    /** @return true if the result is smaller than the rhs. */
    bool operator < ( const T& rhs ) { return wait() < rhs; }

    /** @return true if the result is bigger than the rhs. */
    bool operator > ( const T& rhs ) { return wait() > rhs; }

    /** @return true if the result is smaller or equal. */
    bool operator <= ( const T& rhs ) { return wait() <= rhs; }

    /** @return true if the result is bigger or equal. */
    bool operator >= ( const T& rhs ) { return wait() >= rhs; }
    //@}

protected:
    Impl impl_;
};

typedef Future< bool > f_bool_t; //!< A boolean future

/** Future template specialization for void */
template<> class Future< void >
{
private:
    typedef void (Future< void >::*bool_t)() const;
    void bool_true() const {}

public:
    typedef RefPtr< FutureImpl< void > > Impl; //!< The wait implementation

    /** Construct a new future. */
    explicit Future( Impl impl ) : impl_( impl ){}

    /** Destruct the future. */
     ~Future(){}

    /**
     * Wait for the promise to be fullfilled.
     *
     * @param timeout_ optional timeout in milliseconds. If the future is
     *                 not ready when the timer goes off a timeout exception
     *                 is thrown.
     */
    void wait( const uint32_t timeout_ = LB_TIMEOUT_INDEFINITE )
    {
        impl_->wait( timeout_ );
    }

    /**
     * @return true if the future has been fulfilled, false if it is pending.
     */
    bool isReady() const { return impl_->isReady(); }

protected:
    Impl impl_;
};
}
#endif //LUNCHBOX_FUTURE_H
