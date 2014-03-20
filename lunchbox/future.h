
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

namespace lunchbox
{

/** Base class to implement the wait method fulfilling the future. */
template< class T >
class FutureImpl : public Referenced, public boost::noncopyable
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

    /**
     * @return true if the future has been fulfilled, false if it is pending.
     * @version 1.9.1
     */
    virtual bool isReady() const = 0;
};

/** A future represents a asynchronous operation. Do not subclass. */
template< class T > class Future
{
    typedef void (Future< T >::*bool_t)() const;
    void bool_true() const {}

public:
    typedef RefPtr< FutureImpl< T > > Impl; //!< The wait implementation

    /** Construct a new future. @version 1.9.1 */
    explicit Future( Impl impl ) : impl_( impl ){}

    /** Destruct the future. @version 1.9.1 */
     ~Future(){}

    /** Wait for the promise to be fullfilled. @version 1.9.1 */
    T wait() { return impl_->wait(); }

    /**
     * @return true if the future has been fulfilled, false if it is pending.
     * @version 1.9.1
     */
    bool isReady() const { return impl_->isReady(); }

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
    bool operator > ( const T& rhs ) { return wait() > rhs; }

    /** @return true if the result is smaller or equal. @version 1.9.1 */
    bool operator <= ( const T& rhs ) { return wait() <= rhs; }

    /** @return true if the result is bigger or equal. @version 1.9.1 */
    bool operator >= ( const T& rhs ) { return wait() >= rhs; }
    //@}

protected:
    Impl impl_;
};

}
#endif //LUNCHBOX_FUTURE_H
