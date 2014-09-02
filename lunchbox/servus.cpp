
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

#include "debug.h"

#ifdef LUNCHBOX_USE_DNSSD
#  ifdef _MSC_VER
#    define SERVUS_BONJOUR
#    include "os.h"
#  else
#    include <arpa/inet.h>
#    include <sys/time.h>
#    include <unistd.h>
#  endif
#  ifdef __APPLE__
#    define SERVUS_BONJOUR
#  endif
#  ifndef SERVUS_BONJOUR
#    define SERVUS_AVAHI
#    include "lock.h"
#    include "scopedMutex.h"
#  endif
#  include <dns_sd.h>
#endif

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

namespace lunchbox
{
typedef std::map< std::string, std::string > ValueMap;
typedef std::map< std::string, ValueMap > InstanceMap;
typedef ValueMap::const_iterator ValueMapCIter;
typedef InstanceMap::const_iterator InstanceMapCIter;
static const std::string empty_;

#ifdef SERVUS_AVAHI
static Lock lock_;
#endif

#define ANNOUNCE_TIMEOUT 1000 /*ms*/

namespace detail
{
class Servus
{
public:
#ifdef LUNCHBOX_USE_DNSSD
    explicit Servus( const std::string& name )
        : _name( name )
        , _out( 0 )
        , _in( 0 )
        , _result( lunchbox::Servus::Result::PENDING )
    {}
#else
    explicit Servus( const std::string& ) {}
#endif

    ~Servus()
    {
#ifdef LUNCHBOX_USE_DNSSD
        endBrowsing();
        withdraw();
#endif
    }

#ifdef LUNCHBOX_USE_DNSSD
    void set( const std::string& key, const std::string& value )
    {
        _data[ key ] = value;
        updateRecord_();
    }
#else
    void set( const std::string&, const std::string& ) {}
#endif

    Strings getKeys() const
    {
        Strings keys;
#ifdef LUNCHBOX_USE_DNSSD
        for( ValueMapCIter i = _data.begin(); i != _data.end(); ++i )
            keys.push_back( i->first );
#endif
        return keys;
    }

#ifdef LUNCHBOX_USE_DNSSD
    const std::string& get( const std::string& key ) const
    {
        ValueMapCIter i = _data.find( key );
        if( i != _data.end( ))
            return i->second;
        return empty_;
    }
#else
    const std::string& get( const std::string& ) const
    {
        return empty_;
    }
#endif

    lunchbox::Servus::Result announce( const unsigned short port LB_UNUSED,
                                       const std::string& instance LB_UNUSED )
    {
#ifdef LUNCHBOX_USE_DNSSD
        if( _out )
            return lunchbox::Servus::Result( kDNSServiceErr_AlreadyRegistered );

        TXTRecordRef record;
        createTXTRecord_( record );

        const lunchbox::Servus::Result result(
            DNSServiceRegister( &_out, 0 /* flags */,
                                0 /* all interfaces */,
                                instance.empty() ? 0 : instance.c_str(),
                                _name.c_str(), 0 /* default domains */,
                                0 /* hostname */, htons( port ),
                                TXTRecordGetLength( &record ),
                                TXTRecordGetBytesPtr( &record ),
                                (DNSServiceRegisterReply)registerCBS_,
                                this ));
        TXTRecordDeallocate( &record );

        if( result )
            return _handleEvents( _out, ANNOUNCE_TIMEOUT );

        LBWARN << "DNSServiceRegister returned: " << result << std::endl;
        return result;
#else
        return lunchbox::Servus::Result(
            lunchbox::Servus::Result::NOT_SUPPORTED );
#endif
    }

    void withdraw()
    {
#ifdef LUNCHBOX_USE_DNSSD
        if( !_out )
            return;

        DNSServiceRefDeallocate( _out );
        _out = 0;
#endif
    }

    bool isAnnounced() const
    {
#ifdef LUNCHBOX_USE_DNSSD
        return _out != 0;
#endif
        return false;
    }

    lunchbox::Servus::Result beginBrowsing(
        const lunchbox::Servus::Interface interface_ LB_UNUSED )
    {
#ifdef LUNCHBOX_USE_DNSSD
        if( _in )
            return lunchbox::Servus::Result( kDNSServiceErr_AlreadyRegistered );

        _instanceMap.clear();
        _hosts.clear();

#   ifdef SERVUS_BONJOUR
        const lunchbox::Servus::Interface addr = interface_;
#   else
        // no kDNSServiceInterfaceIndexLocalOnly support in avahi
        // let's implement InterfaceIndexLocalOnly for avahi
        if( interface_ == lunchbox::Servus::IF_LOCAL )
        {
            char hostname[256] = {0};
            gethostname( hostname, 256 );

            std::string name = hostname;
            const size_t dotPos = name.find( '.' );
            if( dotPos != std::string::npos )
                name = name.substr( 0, dotPos );

            _hosts.push_back( name );
            _hosts.push_back( name + ".local." );
        }
        const lunchbox::Servus::Interface addr = lunchbox::Servus::IF_ALL;
#   endif
        const DNSServiceErrorType error = DNSServiceBrowse( &_in, 0, addr,
                                                            _name.c_str(), "",
                                              (DNSServiceBrowseReply)browseCBS_,
                                                            this );
        if( error != kDNSServiceErr_NoError )
        {
            LBWARN << "DNSServiceDiscovery error: " << error << std::endl;
            endBrowsing();
        }
        return lunchbox::Servus::Result( error );
#else
        return lunchbox::Servus::Result(
            lunchbox::Servus::Result::NOT_SUPPORTED );
#endif
    }

    lunchbox::Servus::Result browse( int32_t timeout )
    {
        return _handleEvents( _in, timeout );
    }

    void endBrowsing()
    {
#ifdef LUNCHBOX_USE_DNSSD
        if( !_in )
            return;

        DNSServiceRefDeallocate( _in );
        _in = 0;
#endif
    }

    bool isBrowsing() const
    {
#ifdef LUNCHBOX_USE_DNSSD
        return _in != 0;
#endif
        return false;
    }

    Strings discover( const lunchbox::Servus::Interface interface_ LB_UNUSED,
                      const unsigned browseTime LB_UNUSED )
    {
#ifdef LUNCHBOX_USE_DNSSD
        const lunchbox::Servus::Result& result = beginBrowsing( interface_ );
        if( !result && result != kDNSServiceErr_AlreadyRegistered )
            return getInstances();

        LBASSERT( _in );
        browse( browseTime );
        if( result != kDNSServiceErr_AlreadyRegistered )
            endBrowsing();
#endif
        return getInstances();
    }

    Strings getInstances() const
    {
        Strings instances;
#ifdef LUNCHBOX_USE_DNSSD
        for( InstanceMapCIter i = _instanceMap.begin();
             i != _instanceMap.end(); ++i )
        {
            instances.push_back( i->first );
        }
#endif
        return instances;
    }

#ifdef LUNCHBOX_USE_DNSSD
    Strings getKeys( const std::string& instance ) const
    {
        Strings keys;
        InstanceMapCIter i = _instanceMap.find( instance );
        if( i == _instanceMap.end( ))
            return keys;

        const ValueMap& values = i->second;
        for( ValueMapCIter j = values.begin(); j != values.end(); ++j )
            keys.push_back( j->first );
        return keys;
    }
#else
    Strings getKeys( const std::string& ) const
    {
        return Strings();
    }
#endif

#ifdef LUNCHBOX_USE_DNSSD
    bool containsKey( const std::string& instance,
                      const std::string& key ) const
    {
        InstanceMapCIter i = _instanceMap.find( instance );
        if( i == _instanceMap.end( ))
            return false;

        const ValueMap& values = i->second;
        ValueMapCIter j = values.find( key );
        if( j == values.end( ))
            return false;
        return true;
    }
#else
    bool containsKey( const std::string&, const std::string& ) const
    {
        return false;
    }
#endif

#ifdef LUNCHBOX_USE_DNSSD
    const std::string& get( const std::string& instance,
                            const std::string& key ) const
    {
        InstanceMapCIter i = _instanceMap.find( instance );
        if( i == _instanceMap.end( ))
            return empty_;

        const ValueMap& values = i->second;
        ValueMapCIter j = values.find( key );
        if( j == values.end( ))
            return empty_;
        return j->second;
    }
#else
    const std::string& get( const std::string&, const std::string& ) const
    {
        return empty_;
    }
#endif

#ifdef LUNCHBOX_USE_DNSSD
    void getData( lunchbox::Servus::Data& data )
    {
        data = _instanceMap;
    }
#else
    void getData( lunchbox::Servus::Data& ) {}
#endif

private:
#ifdef LUNCHBOX_USE_DNSSD
    const std::string _name;
    InstanceMap _instanceMap; //!< last discovered data
    ValueMap _data;   //!< self data to announce
    DNSServiceRef _out; //!< used for announce()
    DNSServiceRef _in; //!< used to browse()
    int32_t _result;
    Strings _hosts;
    std::string _browsedName;

    void updateRecord_()
    {
        if( !_out )
            return;

        TXTRecordRef record;
        createTXTRecord_( record );

        const DNSServiceErrorType error =
            DNSServiceUpdateRecord( _out, 0, 0,
                                    TXTRecordGetLength( &record ),
                                    TXTRecordGetBytesPtr( &record ), 0 );
        TXTRecordDeallocate( &record );
        if( error != kDNSServiceErr_NoError )
            LBWARN << "DNSServiceUpdateRecord error: " << error << std::endl;
    }

    void createTXTRecord_( TXTRecordRef& record )
    {
        TXTRecordCreate( &record, 0, 0 );
        for( ValueMapCIter i = _data.begin(); i != _data.end(); ++i )
        {
            const std::string& key = i->first;
            const std::string& value = i->second;
            const uint8_t valueSize = value.length() > 255 ?
                                          255 : uint8_t( value.length( ));
            TXTRecordSetValue( &record, key.c_str(), valueSize, value.c_str( ));
        }
    }

    lunchbox::Servus::Result _handleEvents( DNSServiceRef service,
                                            const int32_t timeout = -1 )
    {
        LBASSERT( service );
        if( !service )
            return lunchbox::Servus::Result( kDNSServiceErr_Unknown );

        const int fd = DNSServiceRefSockFD( service );
        const int nfds = fd + 1;

        LBASSERT( fd >= 0 );
        if( fd < 0 )
            return lunchbox::Servus::Result( kDNSServiceErr_BadParam );

        while( _result == lunchbox::Servus::Result::PENDING )
        {
            fd_set fdSet;
            FD_ZERO( &fdSet );
            FD_SET( fd, &fdSet );

            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;

            const int result = ::select( nfds, &fdSet, 0, 0,
                                         timeout < 0 ? 0 : &tv );
            switch( result )
            {
              case 0: // timeout
                _result = kDNSServiceErr_NoError;
                break;

              case -1: // error
                LBWARN << "Select error: " << strerror( errno ) << " (" << errno
                       << ")" << std::endl;
                if( errno != EINTR )
                {
                    withdraw();
                    _result = errno;
                }
                break;

              default:
                if( FD_ISSET( fd, &fdSet ))
                {
                    const DNSServiceErrorType error =
                        DNSServiceProcessResult( service );

                    if( error != kDNSServiceErr_NoError )
                    {
                        LBWARN << "DNSServiceProcessResult error: " << error
                               << std::endl;
                        withdraw();
                        _result = error;
                    }
                }
                break;
            }
        }

        const lunchbox::Servus::Result result( _result );
        _result = lunchbox::Servus::Result::PENDING; // reset for next operation
        return result;
    }

    static void registerCBS_( DNSServiceRef, DNSServiceFlags,
                              DNSServiceErrorType error, const char* name,
                              const char* type, const char* domain,
                              Servus* servus )
    {
        servus->registerCB_( name, type, domain, error );
    }

    void registerCB_( const char* name, const char* type, const char* domain,
                      DNSServiceErrorType error )
    {
        if( error == kDNSServiceErr_NoError)
            LBINFO << "Registered " << name << "." << type << "." << domain
                      << std::endl;
        else
        {
            LBWARN << "Register callback error: " << error << std::endl;
            withdraw();
        }
        _result = error;
    }

    static void browseCBS_( DNSServiceRef, DNSServiceFlags flags,
                            uint32_t interfaceIdx, DNSServiceErrorType error,
                            const char* name, const char* type,
                            const char* domain, Servus* servus )
    {
        servus->browseCB_( flags, interfaceIdx, error, name, type, domain );
    }

    void browseCB_( DNSServiceFlags flags, uint32_t interfaceIdx,
                    DNSServiceErrorType error, const char* name,
                    const char* type, const char* domain )
    {
        if( error != kDNSServiceErr_NoError)
        {
            LBWARN << "Browse callback error: " << error << std::endl;
            return;
        }

        if( !( flags & kDNSServiceFlagsAdd ))
            return;

        _browsedName = name;

        DNSServiceRef service = 0;
        const DNSServiceErrorType resolve =
            DNSServiceResolve( &service, 0, interfaceIdx, name, type, domain,
                               (DNSServiceResolveReply)resolveCBS_, this );
        if( resolve != kDNSServiceErr_NoError)
            LBWARN << "DNSServiceResolve error: " << resolve << std::endl;

        if( service )
        {
            _handleEvents( service, 500 );
            DNSServiceRefDeallocate( service );
        }
    }

    static void resolveCBS_( DNSServiceRef, DNSServiceFlags,
                             uint32_t /*interfaceIdx*/,
                             DNSServiceErrorType error,
                             const char* /*name*/, const char* host,
                             uint16_t /*port*/,
                             uint16_t txtLen, const unsigned char* txt,
                             Servus* servus )
    {
        if( error == kDNSServiceErr_NoError)
            servus->resolveCB_( host, txtLen, txt );
        servus->_result = error;
    }

    void resolveCB_( const char* host, uint16_t txtLen,
                     const unsigned char* txt )
    {
        if( !_hosts.empty() &&
            std::find( _hosts.begin(), _hosts.end(), host ) == _hosts.end( ))
        {
            return;
        }

        ValueMap& values = _instanceMap[ _browsedName ];
        values[ "servus_host" ] = host;

        char key[256] = {0};
        const char* value = 0;
        uint8_t valueLen = 0;

        uint16_t i = 0;
        while( TXTRecordGetItemAtIndex( txtLen, txt, i, sizeof( key ), key,
                                        &valueLen, (const void**)( &value )) ==
               kDNSServiceErr_NoError )
        {

            values[ key ] = std::string( value, valueLen );
            ++i;
        }
    }
#endif
};
}

Servus::Servus( const std::string& name )
        : _impl( new detail::Servus( name ))
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
#ifdef SERVUS_AVAHI
    ScopedWrite mutex( lock_ );
#endif
    return _impl->announce( port, instance );
}

void Servus::withdraw()
{
#ifdef SERVUS_AVAHI
    ScopedWrite mutex( lock_ );
#endif
    _impl->withdraw();
}

bool Servus::isAnnounced() const
{
    return _impl->isAnnounced();
}

Strings Servus::discover( const Interface addr, const unsigned browseTime )
{
#ifdef SERVUS_AVAHI
    ScopedWrite mutex( lock_ );
#endif
    return _impl->discover( addr, browseTime );
}

Servus::Result Servus::beginBrowsing( const lunchbox::Servus::Interface addr )
{
    return _impl->beginBrowsing( addr );
}

Servus::Result Servus::browse( int32_t timeout )
{
#ifdef SERVUS_AVAHI
    ScopedWrite mutex( lock_ );
#endif
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
