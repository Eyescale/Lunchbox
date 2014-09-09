
/* Copyright (c) 2012-2014, Stefan Eilemann <eile@eyescale.ch>
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#ifndef LUNCHBOX_SERVUS_H
#define LUNCHBOX_SERVUS_H

#include <lunchbox/api.h>
#include <lunchbox/result.h> // nested base class
#include <lunchbox/types.h>
#include <boost/noncopyable.hpp>
#include <map>

namespace lunchbox
{
namespace detail { class Servus; }

/**
 * Simple wrapper for ZeroConf key/value pairs.
 *
 * The servus class allows simple announcement and discovery of key/value pairs
 * using ZeroConf networking. The same instance can be used to announce and/or
 * to browse a ZeroConf service. If the Lunchbox library is compiled without
 * zeroconf support (@sa isAvailable()), this class does not do anything useful.
 *
 * Example: @include tests/servus.cpp
 */
class Servus : public boost::noncopyable
{
public:
    enum Interface
    {
        IF_ALL = 0, //!< use all interfaces
        // (uint32_t) -1 == kDNSServiceInterfaceIndexLocalOnly
        IF_LOCAL = (unsigned)(-1) //!< only local interfaces
    };

    /**
     * The ZeroConf operation result code.
     *
     * The result code is either one of kDNSServiceErr_ or one of static
     * constants defined by this class
     */
    class Result : public lunchbox::Result
    {
    public:
        explicit Result( const int32_t code ) : lunchbox::Result( code ){}
        virtual ~Result(){}
        LUNCHBOX_API std::string getString() const override;

        /** operation did not complete. */
        static const int32_t PENDING = -1;
        /** Lunchbox compiled without ZeroConf support. */
        static const int32_t NOT_SUPPORTED = -2;
        /** Error during polling for event. */
        static const int32_t POLL_ERROR = -3;
    };

    /** @return true if a usable implementation is available. @version 1.9.2 */
    LUNCHBOX_API static bool isAvailable();

    /**
     * Create a new service handle.
     *
     * @param name the service descriptor, e.g., "_hwsd._tcp"
     * @version 0.9
     */
    LUNCHBOX_API explicit Servus( const std::string& name );

    /** Destruct this service. */
    LUNCHBOX_API virtual ~Servus();

    /**
     * Set a key/value pair to be announced.
     *
     * Keys should be at most eight characters, and values are truncated to 255
     * characters. The total length of all keys and values cannot exceed 65535
     * characters. Setting a value on an announced service causes an update
     * which needs some time to propagate after this function returns, that is,
     * calling discover() immediately afterwards will very likely not contain
     * the new key/value pair.
     *
     * @version 0.9
     */
    LUNCHBOX_API void set( const std::string& key, const std::string& value );

    /** @return all (to be) announced keys. @version 1.5.1 */
    LUNCHBOX_API Strings getKeys() const;

    /** @return the value to the given (to be) announced key. @version 1.5.1 */
    LUNCHBOX_API const std::string& get( const std::string& key ) const;

    /**
     * Start announcing the registered key/value pairs.
     *
     * @param port the service IP port in host byte order.
     * @param instance a host-unique instance name, hostname is used if empty.
     * @return the success status of the operation.
     * @version 0.9
     */
    LUNCHBOX_API Result announce( const unsigned short port,
                                  const std::string& instance );

    /** Stop announcing the registered key/value pairs. @version 0.9 */
    LUNCHBOX_API void withdraw();

    /** @return true if the local data is announced. @version 0.9 */
    LUNCHBOX_API bool isAnnounced() const;

    /**
     * Discover all announced key/value pairs.
     *
     * @param addr the scope of the discovery
     * @param browseTime the browse time, in milliseconds, to wait for new
     *                   records.
     * @return all instance names found during discovery.
     * @version 0.9
     * @sa beginBrowsing(), browse(), endBrowsing()
     */
    LUNCHBOX_API Strings discover( const Interface addr,
                                   const unsigned browseTime );

    /**
     * Begin the discovery of announced key/value pairs.
     *
     * @param addr the scope of the discovery
     * @return the success status of the operation.
     * @version 1.9.2
     */
    LUNCHBOX_API Result beginBrowsing( const lunchbox::Servus::Interface addr );

    /**
     * Browse and process discovered key/value pairs.
     *
     * @param timeout The time to spend browsing.
     * @return the success status of the operation.
     * @version 1.9.2
     */
    LUNCHBOX_API Result browse( int32_t timeout = -1 );

    /** Stop a discovery process and return all results. @version 1.9.2 */
    LUNCHBOX_API void endBrowsing();

    /** @return true if the local data is browsing. @version 1.9.2 */
    LUNCHBOX_API bool isBrowsing() const;

    /** @return all instances found during the last discovery. @version 0.9 */
    LUNCHBOX_API Strings getInstances() const;

    /** @return all keys discovered on the given instance. @version 0.9 */
    LUNCHBOX_API Strings getKeys( const std::string& instance ) const;

    /** @return true if the given key was discovered. @version 0.9 */
    LUNCHBOX_API bool containsKey( const std::string& instance,
                                   const std::string& key ) const;

    /** @return the value of the given key and instance. @version 0.9 */
    LUNCHBOX_API const std::string& get( const std::string& instance,
                                         const std::string& key ) const;

    /** @internal */
    typedef std::map< std::string, std::map< std::string, std::string > > Data;

    /** @internal */
    LUNCHBOX_API void getData( Data& data );

private:
    detail::Servus* const _impl;
    friend std::ostream& operator << ( std::ostream&, const Servus& );
};

/** Output the servus instance in human-readable format. @version 1.5.1 */
LUNCHBOX_API std::ostream& operator << ( std::ostream&, const Servus& );

/** Output the servus interface in human-readable format. @version 1.9.2 */
LUNCHBOX_API std::ostream& operator << (std::ostream&,const Servus::Interface&);
}

#endif // LUNCHBOX_SERVUS_H
