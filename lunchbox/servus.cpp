
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

#include "servus.h"


namespace lunchbox
{
typedef std::map< std::string, std::string > ValueMap;
typedef std::map< std::string, ValueMap > InstanceMap;
typedef ValueMap::const_iterator ValueMapCIter;
typedef InstanceMap::const_iterator InstanceMapCIter;

#define ANNOUNCE_TIMEOUT 1000 /*ms*/

namespace detail
{
class Servus
{
public:
    Servus() {}
    virtual ~Servus() {}
    virtual void set( const std::string& key, const std::string& value ) = 0;
    virtual Strings getKeys() const = 0;
    virtual const std::string& get( const std::string& key ) const = 0;

    virtual lunchbox::Servus::Result announce( const unsigned short port,
                                               const std::string& instance ) =0;
    virtual void withdraw() = 0;
    virtual bool isAnnounced() const = 0;

    virtual lunchbox::Servus::Result beginBrowsing(
        const lunchbox::Servus::Interface interface_ ) = 0;
    virtual lunchbox::Servus::Result browse( int32_t timeout ) = 0;

    virtual void endBrowsing() = 0;
    virtual bool isBrowsing() const = 0;
    virtual Strings discover( const lunchbox::Servus::Interface interface_,
                              const unsigned browseTime ) = 0;

    virtual Strings getInstances() const = 0;
    virtual Strings getKeys( const std::string& instance ) const = 0;
    virtual bool containsKey( const std::string& instance,
                              const std::string& key ) const = 0;
    virtual const std::string& get( const std::string& instance,
                                    const std::string& key ) const = 0;

    virtual void getData( lunchbox::Servus::Data& data ) = 0;
};

}
}

// Impls need detail interface definition above
#ifdef LUNCHBOX_USE_DNSSD
#  include "dnssd/servus.h"
#else
#  include "none/servus.h"
#endif

namespace lunchbox
{
Servus::Servus( const std::string& name )
#ifdef LUNCHBOX_USE_DNSSD
    : _impl( new dnssd::Servus( name ))
#else
    : _impl( new none::Servus( ))
#endif
{}

Servus::~Servus()
{
    delete _impl;
}

std::string Servus::Result::getString() const
{
    const int32_t code = getCode();
    switch( code )
    {
#ifdef LUNCHBOX_USE_DNSSD
    case kDNSServiceErr_Unknown:           return "unknown error";
    case kDNSServiceErr_NoSuchName:        return "name not found";
    case kDNSServiceErr_NoMemory:          return "out of memory";
    case kDNSServiceErr_BadParam:          return "bad parameter";
    case kDNSServiceErr_BadReference:      return "bad reference";
    case kDNSServiceErr_BadState:          return "bad state";
    case kDNSServiceErr_BadFlags:          return "bad flags";
    case kDNSServiceErr_Unsupported:       return "unsupported";
    case kDNSServiceErr_NotInitialized:    return "not initialized";
    case kDNSServiceErr_AlreadyRegistered: return "already registered";
    case kDNSServiceErr_NameConflict:      return "name conflict";
    case kDNSServiceErr_Invalid:           return "invalid value";
    case kDNSServiceErr_Firewall:          return "firewall";
    case kDNSServiceErr_Incompatible:
        return "client library incompatible with daemon";
    case kDNSServiceErr_BadInterfaceIndex: return "bad interface index";
    case kDNSServiceErr_Refused:           return "refused";
    case kDNSServiceErr_NoSuchRecord:      return "no such record";
    case kDNSServiceErr_NoAuth:            return "no authentication";
    case kDNSServiceErr_NoSuchKey:         return "no such key";
    case kDNSServiceErr_NATTraversal:      return "NAT traversal";
    case kDNSServiceErr_DoubleNAT:         return "double NAT";
    case kDNSServiceErr_BadTime:           return "bad time";
#endif

    case PENDING:          return "operation did not complete";
    case NOT_SUPPORTED:    return "Lunchbox compiled without ZeroConf support";

    default:
        if( code > 0 )
            return ::strerror( code );
        return lunchbox::Result::getString();
    }
}

void Servus::set( const std::string& key, const std::string& value )
{
    _impl->set( key, value );
}

Strings Servus::getKeys() const
{
    return _impl->getKeys();
}

const std::string& Servus::get( const std::string& key ) const
{
    return _impl->get( key );
}

Servus::Result Servus::announce( const unsigned short port,
                                 const std::string& instance )
{
    return _impl->announce( port, instance );
}

void Servus::withdraw()
{
    _impl->withdraw();
}

bool Servus::isAnnounced() const
{
    return _impl->isAnnounced();
}

Strings Servus::discover( const Interface addr, const unsigned browseTime )
{
    return _impl->discover( addr, browseTime );
}

Servus::Result Servus::beginBrowsing( const lunchbox::Servus::Interface addr )
{
    return _impl->beginBrowsing( addr );
}

Servus::Result Servus::browse( int32_t timeout )
{
    return _impl->browse( timeout );
}

void Servus::endBrowsing()
{
    _impl->endBrowsing();
}

bool Servus::isBrowsing() const
{
    return _impl->isBrowsing();
}

Strings Servus::getInstances() const
{
    return _impl->getInstances();
}

Strings Servus::getKeys( const std::string& instance ) const
{
    return _impl->getKeys( instance );
}

bool Servus::containsKey( const std::string& instance,
                          const std::string& key ) const
{
    return _impl->containsKey( instance, key );
}

const std::string& Servus::get( const std::string& instance,
                                const std::string& key ) const
{
    return _impl->get( instance, key );
}

void Servus::getData( Data& data )
{
    _impl->getData( data );
}

#ifdef LUNCHBOX_USE_DNSSD
std::ostream& operator << ( std::ostream& os, const Servus& servus )
{
    os << disableFlush << disableHeader << "Servus instance"
       << (servus.isAnnounced() ? " " : " not ") << "announced"
       << (servus.isBrowsing() ? " " : " not ") << "browsing" << indent;

    const Strings& keys = servus.getKeys();
    for( StringsCIter i = keys.begin(); i != keys.end(); ++i )
        os << std::endl << *i << " = " << servus.get( *i );

    return os << exdent << enableHeader << enableFlush;
}
#else
std::ostream& operator << ( std::ostream& os, const Servus& )
{
    return os << "No dnssd support, empty Servus implementation";
}
#endif

}
