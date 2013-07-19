
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

namespace lunchbox
{

/** @internal subclass to implement the wait method fulfilling the future. */
template< class T > class FutureImpl : public Referenced
{
public:
    /** Destruct the future. @version 1.9.1 */
    virtual ~FutureImpl(){}

    /** Wait for the promise to be fullfilled. @version 1.9.1 */
    virtual T wait() = 0;
};

/** A future represents a asynchronous operation. Do not subclass. */
template< class T > class Future
{
public:
    typedef RefPtr< FutureImpl< T > > Impl; //!< The wait implementation

    /** Construct a new future. @version 1.9.1 */
    Future( Impl impl ) : impl_( impl ){}

    /** Destruct the future. @version 1.9.1 */
     ~Future(){}

    /** Wait for the promise to be fullfilled. @version 1.9.1 */
    T wait() { return impl_->wait(); }

private:
    Impl impl_;
};

typedef Future< bool > Futureb; //!< A future with a boolean return value

}
#endif //LUNCHBOX_FUTURE_H
