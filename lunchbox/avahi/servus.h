
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

#include "../clock.h"
#include "../debug.h"

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/simple-watch.h>

#include <net/if.h>

namespace lunchbox
{
namespace avahi
{
class Servus : public detail::Servus
{
public:
    explicit Servus( const std::string& name )
        : _name( name )
        , _poll( avahi_simple_poll_new( ))
        , _client( 0 )
        , _browser( 0 )
        , _group( 0 )
        , _result( lunchbox::Servus::Result::PENDING )
        , _port( 0 )
        , _announcable( false )
    {
        if( !_poll )
            LBTHROW( std::runtime_error( "Can't setup avahi poll device" ));

        int error = 0;
        _client = avahi_client_new( avahi_simple_poll_get( _poll ),
                                    (AvahiClientFlags)(0), _clientCBS, this,
                                    &error );
        if( !_client )
            LBTHROW( std::runtime_error(
                         std::string( "Can't setup avahi client: " ) +
                         avahi_strerror( error )));
    }

    virtual ~Servus()
    {
        withdraw();
        endBrowsing();

        if( _client )
            avahi_client_free( _client );
        if( _poll )
            avahi_simple_poll_free( _poll );
    }

    lunchbox::Servus::Result announce( const unsigned short port,
                                       const std::string& instance ) final
    {
        _result = lunchbox::Servus::Result::PENDING;
        _port = port;
        if( instance.empty( ))
        {
            char cHostname[256] = {0};
            gethostname( cHostname, 256 );
            _announce = cHostname;
        }
        else
            _announce = instance;

        if( _announcable )
            _createServices();
        else
        {
            lunchbox::Clock clock;
            while( !_announcable &&
                   _result == lunchbox::Servus::Result::PENDING &&
                   clock.getTime64() < ANNOUNCE_TIMEOUT )
            {
                avahi_simple_poll_iterate( _poll, ANNOUNCE_TIMEOUT );
            }
        }

        return lunchbox::Servus::Result( _result );
    }

    void withdraw() final
    {
        _announce.clear();
        _port = 0;
        if( _group )
            avahi_entry_group_reset( _group );
    }

    bool isAnnounced() const final
    {
        return ( _group && !avahi_entry_group_is_empty( _group ));
    }

    lunchbox::Servus::Result beginBrowsing( const lunchbox::Servus::Interface )
        final
    {
        if( _browser )
            return lunchbox::Servus::Result( lunchbox::Servus::Result::PENDING);

        _instanceMap.clear();
        return _browse( lunchbox::Servus::IF_ALL );
    }

    lunchbox::Servus::Result browse( const int32_t timeout ) final
    {
        _result = lunchbox::Servus::Result::PENDING;
        lunchbox::Clock clock;

        while( _result == lunchbox::Servus::Result::PENDING &&
               clock.getTime64() < timeout )
        {
            if( avahi_simple_poll_iterate( _poll, timeout ) != 0 )
            {
                if( _result == lunchbox::Result::SUCCESS )
                    _result = lunchbox::Servus::Result::POLL_ERROR;
                break;
            }
        }

        return lunchbox::Servus::Result( _result );
    }

    void endBrowsing() final
    {
        if( _browser )
            avahi_service_browser_free( _browser );
        _browser = 0;
    }

    bool isBrowsing() const final { return _browser; }

    Strings discover( const lunchbox::Servus::Interface addr,
                      const unsigned browseTime ) final
    {
        const lunchbox::Servus::Result& result = beginBrowsing( addr );
        if( !result && result != lunchbox::Servus::Result::PENDING )
            return getInstances();

        LBASSERT( _browser );
        browse( browseTime );
        if( result != lunchbox::Servus::Result::PENDING )
            endBrowsing();
        return getInstances();
    }

private:
    const std::string _name;
    AvahiSimplePoll* _poll;
    AvahiClient* _client;
    AvahiServiceBrowser* _browser;
    AvahiEntryGroup* _group;
    int32_t _result;
    std::string _announce;
    unsigned short _port;
    bool _announcable;

    lunchbox::Servus::Result _browse( const lunchbox::Servus::Interface addr )
    {
        _result = lunchbox::Servus::Result::SUCCESS;
        int ifIndex = AVAHI_IF_UNSPEC;
        if( addr == lunchbox::Servus::IF_LOCAL )
        {
            ifIndex = if_nametoindex( "lo" );
            if( ifIndex == 0 )
            {
                LBWARN << "Can't enumerate loopback interface: "
                       << lunchbox::sysError() << std::endl;
                return lunchbox::Servus::Result( errno );
            }
        }

        LBINFO << "Browse for " << _name << " on " << ifIndex << std::endl;
        _browser = avahi_service_browser_new( _client, ifIndex,
                                              AVAHI_PROTO_UNSPEC, _name.c_str(),
                                              0, (AvahiLookupFlags)(0),
                                              _browseCBS, this );
        if( _browser )
            return lunchbox::Servus::Result( _result );

        _result = avahi_client_errno( _client );
        LBWARN << "Failed to create browser: " << avahi_strerror( _result )
               << std::endl;
        return lunchbox::Servus::Result( _result );
    }

    // Client state change
    static void _clientCBS( AvahiClient*, AvahiClientState state,
                            void* servus )
    {
        ((Servus*)servus)->_clientCB( state );
    }

    void _clientCB( AvahiClientState state )
    {
        LBINFO << "Client state " << int(state) << std::endl;
        switch (state)
        {
        case AVAHI_CLIENT_S_RUNNING:
            _announcable = true;
            if( !_announce.empty( ))
                _createServices();
            break;

        case AVAHI_CLIENT_FAILURE:
            _result = avahi_client_errno( _client );
            LBWARN << "Client failure: " << avahi_strerror( _result )
                   << std::endl;
            avahi_simple_poll_quit( _poll );
            break;

        case AVAHI_CLIENT_S_COLLISION:
            /* Let's drop our registered services. When the server is back in
             * AVAHI_SERVER_RUNNING state we will register them again with the
             * new host name. */

        case AVAHI_CLIENT_S_REGISTERING:
            /* The server records are now being established. This might be
             * caused by a host name change. We need to wait for our own records
             * to register until the host name is properly esatblished. */

            // if( group )
            //     avahi_entry_group_reset( group );
            break;

        case AVAHI_CLIENT_CONNECTING:
            /*nop*/;
        }
    }

    // Browsing
    static void _browseCBS( AvahiServiceBrowser*, AvahiIfIndex ifIndex,
                            AvahiProtocol protocol, AvahiBrowserEvent event,
                            const char* name, const char* type,
                            const char* domain, AvahiLookupResultFlags,
                            void* servus )
    {
        ((Servus*)servus)->_browseCB( ifIndex, protocol, event, name, type,
                                      domain );
    }

    void _browseCB( const AvahiIfIndex ifIndex, const AvahiProtocol protocol,
                    const AvahiBrowserEvent event, const char* name,
                    const char* type, const char* domain )
    {
        LBINFO << "Browse event " << int(event) << " for "
               << (name ? "none" : name) << " type " <<  (type ? "none" : type)
               << std::endl;
        switch( event )
        {
        case AVAHI_BROWSER_FAILURE:
            _result = avahi_client_errno( _client );
            LBWARN << "Browser failure: " << avahi_strerror( _result )
                   << std::endl;
            avahi_simple_poll_quit( _poll );
            break;

        case AVAHI_BROWSER_NEW:
            /* We ignore the returned resolver object. In the callback function
               we free it. If the server is terminated before the callback
               function is called the server will free the resolver for us. */
            if( !avahi_service_resolver_new( _client, ifIndex, protocol, name,
                                             type, domain, AVAHI_PROTO_UNSPEC,
                                             (AvahiLookupFlags)(0),
                                             _resolveCBS, this ))
            {
                _result = avahi_client_errno( _client );
                LBWARN << "Error creating resolver: "
                       << avahi_strerror( _result ) << std::endl;
                avahi_simple_poll_quit( _poll );
                break;
            }

        case AVAHI_BROWSER_REMOVE:
            LBUNIMPLEMENTED;
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            _result = lunchbox::Result::SUCCESS;
            break;
        }
    }

    // Resolving
    static void _resolveCBS( AvahiServiceResolver* resolver,
                             AvahiIfIndex, AvahiProtocol,
                             AvahiResolverEvent event, const char* name,
                             const char* type, const char*,
                             const char*, const AvahiAddress*,
                             uint16_t, AvahiStringList *txt,
                             AvahiLookupResultFlags, void* servus )
    {
        ((Servus*)servus)->_resolveCB( resolver, event, name, type, txt );
    }

    void _resolveCB( AvahiServiceResolver* resolver,
                             const AvahiResolverEvent event, const char* name,
                     const char* type, AvahiStringList *txt )
    {
        LBINFO << "Resolve event " << int(event) << std::endl;
        switch( event )
        {
        case AVAHI_RESOLVER_FAILURE:
            _result = avahi_client_errno( _client );
            LBWARN << "Resolver error: " << avahi_strerror( _result )
                   << std::endl;
            break;

        case AVAHI_RESOLVER_FOUND:
            for( ; txt; txt = txt->next )
                LBINFO << name << " " << type << " " << txt->text
                       << std::endl;
            break;
        }

        avahi_service_resolver_free( resolver );
    }

    // Announcing
    void _updateRecord() final
    {
        if( _announce.empty() || !_announcable )
            return;

        if( _group )
            avahi_entry_group_reset( _group );
        _createServices();
    }

    void _createServices()
    {
        if( !_group )
            _group = avahi_entry_group_new( _client, _groupCBS, this );
        else
            avahi_entry_group_reset( _group );

        if( !_group )
            return;

        AvahiStringList* data = 0;
        for( detail::ValueMapCIter i = _data.begin(); i != _data.end(); ++i )
            data = avahi_string_list_add_pair( data, i->first.c_str(),
                                               i->second.c_str( ));

        _result = avahi_entry_group_add_service_strlst(
            _group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
                (AvahiPublishFlags)(0), _announce.c_str(), _name.c_str(), 0, 0,
                _port, data );

        LBINFO << "announced " << _data.size() << " pairs: "
               << avahi_string_list_to_string( data ) << std::endl;
        if( data )
            avahi_string_list_free( data );

        if( _result != lunchbox::Result::SUCCESS )
        {
            avahi_simple_poll_quit( _poll );
            return;
        }

        _result = avahi_entry_group_commit( _group );
        if( _result != lunchbox::Result::SUCCESS )
            avahi_simple_poll_quit( _poll );
    }

    static void _groupCBS( AvahiEntryGroup*, AvahiEntryGroupState state,
                           void* servus )
    {
        ((Servus*)servus)->_groupCB( state );
    }

    void _groupCB( const AvahiEntryGroupState state )
    {
        switch( state )
        {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            break;

        case AVAHI_ENTRY_GROUP_COLLISION:
        case AVAHI_ENTRY_GROUP_FAILURE:
            _result = EEXIST;
            avahi_simple_poll_quit( _poll );
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            /*nop*/ ;
        }
    }
};

}
}
