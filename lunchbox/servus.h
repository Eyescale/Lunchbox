
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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
#include <lunchbox/types.h>
#include <lunchbox/nonCopyable.h>
#include <map>

namespace lunchbox
{
namespace detail { class Servus; }

/**
 * Simple wrapper for ZeroConf key/value pairs.
 *
 * The servus class allows simple announcement and discovery of key/value
 * pairs using ZeroConf networking. The same instance can be used to announce
 * and/or to browse a ZeroConf service. If the Lunchbox library is compiled
 * without zeroconf support (LUNCHBOX_USE_DNSSD is not set), this class does not
 * do anything useful.
 */
class Servus : public NonCopyable
{
public:
    enum Interface
    {
        IF_ALL = 0, //!< use all interfaces
        // (uint32_t) -1 == kDNSServiceInterfaceIndexLocalOnly
        IF_LOCAL = (unsigned)(-1) //!< only local interfaces
    };

    /**
     * Create a new service handle.
     *
     * @param name the service descriptor, e.g., "_gpu-sd._tcp"
     * @version 0.9
     */
    LUNCHBOX_API Servus( const std::string& name );

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
     * @version 0.9
     */
    LUNCHBOX_API bool announce( const unsigned short port,
                                const std::string& instance );

    /** Stop announcing the registered key/value pairs. @version 0.9 */
    LUNCHBOX_API void withdraw();

    /** @return true if the local data is announced. @version 0.9 */
    LUNCHBOX_API bool isAnnounced() const;

    /**
     * Discover all announced key/value pairs.
     *
     * @param interface the scope of the discovery
     * @param browseTime the browse time, in millisecond, for discovery.
     * @return all instance names found during discovery.
     * @version 0.9
     */
    LUNCHBOX_API Strings discover( const Interface addr,
                                   const unsigned browseTime );

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
    detail::Servus* const impl_;
};

/** Output the servus instance in human-readable format. @version 1.5.1 */
LUNCHBOX_API std::ostream& operator << ( std::ostream&, const Servus& );
}

#endif // LUNCHBOX_SERVUS_H
