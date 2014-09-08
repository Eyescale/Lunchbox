
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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

#include "../debug.h"
#include "../scopedMutex.h"
#ifdef LUNCHBOX_USE_DNSSD
#  ifdef _MSC_VER
#    define SERVUS_BONJOUR
#    include "../os.h"
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
#  endif
#  include <dns_sd.h>
#endif

#include <algorithm>

namespace lunchbox
{
namespace dnssd
{
static const std::string empty_;
#ifdef SERVUS_AVAHI
static Lock _lock;
#endif

class Servus : public detail::Servus
{
public:
    explicit Servus( const std::string& name )
        : _name( name )
        , _out( 0 )
        , _in( 0 )
        , _result( lunchbox::Servus::Result::PENDING )
    {}

    virtual ~Servus()
    {
        endBrowsing();
        withdraw();
    }

    void set( const std::string& key, const std::string& value ) final
    {
        _data[ key ] = value;
        updateRecord_();
    }

    Strings getKeys() const final
    {
        Strings keys;
        for( ValueMapCIter i = _data.begin(); i != _data.end(); ++i )
            keys.push_back( i->first );
        return keys;
    }

    const std::string& get( const std::string& key ) const final
    {
        ValueMapCIter i = _data.find( key );
        if( i != _data.end( ))
            return i->second;
        return empty_;
    }

    lunchbox::Servus::Result announce( const unsigned short port,
                                       const std::string& instance ) final
    {
#ifdef SERVUS_AVAHI
        ScopedWrite mutex( _lock );
#endif
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
    }

    void withdraw() final
    {
#ifdef SERVUS_AVAHI
        ScopedWrite mutex( _lock );
#endif
        _withdraw();
    }

    bool isAnnounced() const final
    {
        return _out != 0;
    }

    lunchbox::Servus::Result beginBrowsing(
        const lunchbox::Servus::Interface interface_ ) final
    {
        if( _in )
            return lunchbox::Servus::Result( kDNSServiceErr_AlreadyRegistered );

        _instanceMap.clear();
        _hosts.clear();

#ifdef SERVUS_BONJOUR
        return _browse( interface_ );
#else
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
        return _browse( lunchbox::Servus::IF_ALL );
#endif
    }

    lunchbox::Servus::Result browse( int32_t timeout ) final
    {
#ifdef SERVUS_AVAHI
        // The dns-sd compat layer of avahi does not send remove events.
        // Let's simply restart. Consider using the core avahi API!
        endBrowsing();
        _instanceMap.clear();
        _hosts.clear();

        const lunchbox::Servus::Result result =
            _browse( lunchbox::Servus::IF_ALL );
        if( !result )
            return result;
#endif
        return _handleEvents( _in, timeout );
    }

    void endBrowsing() final
    {
        if( !_in )
            return;

        DNSServiceRefDeallocate( _in );
        _in = 0;
    }

    bool isBrowsing() const final
    {
        return _in != 0;
    }

    Strings discover( const lunchbox::Servus::Interface interface_,
                      const unsigned browseTime ) final
    {
#ifdef SERVUS_AVAHI
        ScopedWrite mutex( _lock );
#endif
        const lunchbox::Servus::Result& result = beginBrowsing( interface_ );
        if( !result && result != kDNSServiceErr_AlreadyRegistered )
            return getInstances();

        LBASSERT( _in );
        browse( browseTime );
        if( result != kDNSServiceErr_AlreadyRegistered )
            endBrowsing();
        return getInstances();
    }

    Strings getInstances() const final
    {
        Strings instances;
        for( InstanceMapCIter i = _instanceMap.begin();
             i != _instanceMap.end(); ++i )
        {
            instances.push_back( i->first );
        }
        return instances;
    }

    Strings getKeys( const std::string& instance ) const final
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

    bool containsKey( const std::string& instance,
                      const std::string& key ) const final
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

    const std::string& get( const std::string& instance,
                            const std::string& key ) const final
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

    void getData( lunchbox::Servus::Data& data ) final
    {
        data = _instanceMap;
    }

private:
    const std::string _name;
    InstanceMap _instanceMap; //!< last discovered data
    ValueMap _data;   //!< self data to announce
    DNSServiceRef _out; //!< used for announce()
    DNSServiceRef _in; //!< used to browse()
    int32_t _result;
    Strings _hosts;
    std::string _browsedName;


    void _withdraw()
    {
        if( !_out )
            return;

        DNSServiceRefDeallocate( _out );
        _out = 0;
    }

    lunchbox::Servus::Result _browse( const lunchbox::Servus::Interface addr )
    {
        const DNSServiceErrorType error =
            DNSServiceBrowse( &_in, 0, addr, _name.c_str(), "",
                              (DNSServiceBrowseReply)browseCBS_, this );

        if( error != kDNSServiceErr_NoError )
        {
            LBWARN << "DNSServiceDiscovery error: " << error << std::endl;
            endBrowsing();
        }
        return lunchbox::Servus::Result( error );
    }

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
                    _withdraw();
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
                        _withdraw();
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
            _withdraw();
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

        if( flags & kDNSServiceFlagsAdd )
        {
            _browsedName = name;

            DNSServiceRef service = 0;
            const DNSServiceErrorType resolve =
                DNSServiceResolve( &service, 0, interfaceIdx, name, type,
                                   domain, (DNSServiceResolveReply)resolveCBS_,
                                   this );
            if( resolve != kDNSServiceErr_NoError)
                LBWARN << "DNSServiceResolve error: " << resolve << std::endl;

            if( service )
            {
                _handleEvents( service, 500 );
                DNSServiceRefDeallocate( service );
            }
        }
        else // dns_sd.h: callback with the Add flag NOT set indicates a Remove
            _instanceMap.erase( name );
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
            LBVERB << "Ignore non-local record from " << host << std::endl;
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
};
}
}
