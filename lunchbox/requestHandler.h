
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_REQUESTHANDLER_H
#define LUNCHBOX_REQUESTHANDLER_H

#include <lunchbox/api.h>       // LUNCHBOX_API definition
#include <lunchbox/thread.h>    // thread-safety macros
#include <lunchbox/types.h>
#include <servus/uint128_t.h>

namespace lunchbox
{
namespace detail { class RequestHandler; }

/**
 * A thread-safe request handler.
 *
 * Different execution threads can synchronize using a request handler. One
 * thread registers a request, and later waits for the request to be
 * served. Another thread can serve the request, providing a result value.
 *
 * Thread-safety: The methods registerRequest(), unregisterRequest() and
 * waitRequest() are supposed to be called from one 'waiting' thread, and the
 * functions serveRequest() and deleteRequest() are supposed to be called only
 * from one 'serving' thread.
 *
 * Example: @include tests/requestHandler.cpp
 */
class RequestHandler : public boost::noncopyable
{
public:
    /** Construct a new request handler.  @version 1.0 */
    LUNCHBOX_API RequestHandler();

    /** Destruct the request handler. */
    LUNCHBOX_API ~RequestHandler();

    /**
     * Register a request.
     *
     * @param data a pointer to user-specific data for the request, can be 0.
     * @return A Future which will be fulfilled on serveRequest().
     * @version 1.9.1
     */
    template< class T > Request< T > registerRequest( void* data = 0 );

    /**
     * Register a request.
     *
     * @param data a pointer to user-specific data for the request, can be 0.
     * @return the request identifier.
     * @version 1.0
     * @deprecated use the future-based registerRequest()
     */
    uint32_t registerRequest( void* data = 0 ) { return _register( data ); }

    /**
     * Unregister a request.
     *
     * Note that waitRequest automatically unregisters the request when it was
     * successful. This method is only used when a waitRequest has timed out and
     * the request will no longer be used.
     *
     * @param requestID the request identifier.
     * @version 1.0
     */
    LUNCHBOX_API void unregisterRequest( const uint32_t requestID );

    /**
     * Wait a given time for the completion of a request.
     *
     * The request is unregistered upon successful completion, i.e, the when the
     * method returns true.
     *
     * @param requestID the request identifier.
     * @param result  the result code of the operation.
     * @param timeout the timeout in milliseconds to wait for the request,
     *                or <code>LB_TIMEOUT_INDEFINITE</code> to wait
     *                indefinitely.
     * @return true if the request was served, false if not.
     * @version 1.0
     */
    LUNCHBOX_API bool waitRequest( const uint32_t requestID, void*& result,
                               const uint32_t timeout = LB_TIMEOUT_INDEFINITE );

    /** Wait for a request with an uint32_t result. @version 1.0 */
    LUNCHBOX_API bool waitRequest( const uint32_t requestID, uint32_t& result,
                               const uint32_t timeout = LB_TIMEOUT_INDEFINITE );
    /** Wait for a request with a bool result. @version 1.0 */
    LUNCHBOX_API bool waitRequest( const uint32_t requestID, bool& result,
                               const uint32_t timeout = LB_TIMEOUT_INDEFINITE );
    /** Wait for a request with an uint128_t result. @version 1.0 */
    LUNCHBOX_API bool waitRequest( const uint32_t requestID,
                                   servus::uint128_t& result,
                               const uint32_t timeout = LB_TIMEOUT_INDEFINITE );
    /** Wait for a request without a result. @version 1.0 */
    LUNCHBOX_API bool waitRequest( const uint32_t requestID );

    /**
     * Poll for the completion of a request.
     *
     * @version 1.0
     * @deprecated use Request::isReady()
     */
    bool isRequestServed( const uint32_t id ) const LB_DEPRECATED
        { return isRequestReady( id ); }

    /**
     * Retrieve the user-specific data for a request.
     *
     * @param requestID the request identifier.
     * @return the user-specific data for the request.
     * @version 1.0
     */
    LUNCHBOX_API void* getRequestData( const uint32_t requestID );

    /**
     * Serve a request with a void* result.
     *
     * @param requestID the request identifier.
     * @param result the result of the request.
     * @version 1.0
     */
    LUNCHBOX_API void serveRequest( const uint32_t requestID,
                                    void* result = 0 );
    /** Serve a request with an uint32_t result. @version 1.0 */
    LUNCHBOX_API void serveRequest( const uint32_t requestID,
                                    uint32_t result );
    /** Serve a request with a bool result. @version 1.0 */
    LUNCHBOX_API void serveRequest( const uint32_t requestID, bool result );
    /** Serve a request with an uint128_t result. @version 1.0 */
    LUNCHBOX_API void serveRequest( const uint32_t requestID,
                                    const servus::uint128_t& result );
    /**
     * @return true if this request handler has pending requests.
     * @version 1.0
     */
    LUNCHBOX_API bool hasPendingRequests() const;

    LUNCHBOX_API bool isRequestReady( const uint32_t ) const; //!< @internal

private:
    detail::RequestHandler* const _impl;
    friend LUNCHBOX_API std::ostream& operator << ( std::ostream&,
                                                    const RequestHandler& );
    LUNCHBOX_API uint32_t _register( void* data );
    LB_TS_VAR( _thread );
};

LUNCHBOX_API std::ostream& operator << ( std::ostream&, const RequestHandler& );
}

#include <lunchbox/request.h>

namespace lunchbox
{
template< class T > inline
Request< T > RequestHandler::registerRequest( void* data )
{
    return Request< T >( *this, _register( data ));
}
}

#endif //LUNCHBOX_REQUESTHANDLER_H
