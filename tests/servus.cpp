
/* Copyright (c) 2012-2014, Stefan.Eilemann@epfl.ch
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#include "test.h"

#include <lunchbox/servus.h>
#include <lunchbox/rng.h>
#include <boost/lexical_cast.hpp>

#ifdef LUNCHBOX_USE_DNSSD
#  include <dns_sd.h>
#endif

int main( int, char** )
{
    lunchbox::RNG rng;
    const uint16_t port = (rng.get< uint16_t >() % 60000) + 1024;
    std::stringstream serviceName;
    serviceName << "_servustest_" << port << "._tcp";

    try
    {
        lunchbox::Servus service( serviceName.str( ));
    }
    catch( const std::runtime_error& e )
    {
        if( getenv( "TRAVIS" ))
        {
            std::cerr << "Bailing, no avahi on a Travis CI setup" << std::endl;
            TEST( e.what() ==
                  std::string( "Can't setup avahi client: Daemon not running"));
            return EXIT_SUCCESS;
        }
        throw e;
    }

    lunchbox::Servus service( serviceName.str( ));
    const lunchbox::Servus::Result& result = service.announce( port,
                                    boost::lexical_cast< std::string >( port ));

    if( !lunchbox::Servus::isAvailable( ))
    {
        TESTINFO( result == lunchbox::Servus::Result::NOT_SUPPORTED, result );
        return EXIT_SUCCESS;
    }

#ifdef LUNCHBOX_USE_DNSSD
    TEST( lunchbox::Result::SUCCESS == kDNSServiceErr_NoError );
#endif
    if( result != lunchbox::Result::SUCCESS ) // happens on CI VMs
    {
        LBWARN << "Bailing, got " << result
               << ": looks like a broken zeroconf setup" << std::endl;
        return EXIT_SUCCESS;
    }
    TESTINFO( result, result );

    service.withdraw();
    service.set( "foo", "bar" );
    TEST( service.announce( port, boost::lexical_cast< std::string >( port )));

    lunchbox::Strings hosts = service.discover( lunchbox::Servus::IF_LOCAL,
                                                2000 );
    if( hosts.empty() && getenv( "TRAVIS" ))
    {
        std::cerr << "Bailing, got no hosts on a Travis CI setup" << std::endl;
        return EXIT_SUCCESS;
    }

    TESTINFO( hosts.size() == 1, hosts.size( ));
    TESTINFO( hosts.front() == boost::lexical_cast< std::string >( port ),
              hosts.front( ));
    TEST( service.get( hosts.front(), "foo" ) == "bar" );
    lunchbox::sleep( 200 );

    service.set( "foobar", "42" );
    lunchbox::sleep( 2000 );
    hosts = service.discover( lunchbox::Servus::IF_LOCAL, 2000 );
    TESTINFO( hosts.size() == 1, hosts.size( ));
    TESTINFO( service.get( hosts.front(), "foobar" ) == "42",
              "Keys: " << lunchbox::format( service.getKeys( hosts.front( ))));
    TEST( service.getKeys().size() == 2 );

    // continuous browse API
    TEST( !service.isBrowsing( ));
    TESTRESULT( service.beginBrowsing( lunchbox::Servus::IF_LOCAL ),
                lunchbox::Servus::Result );
    TEST( service.isBrowsing( ));
    TEST( service.beginBrowsing( lunchbox::Servus::IF_LOCAL ) ==
          lunchbox::Servus::Result::PENDING );
    TEST( service.isBrowsing( ));

    TESTINFO( service.browse( 200 ), service.browse( 0 ));
    hosts = service.getInstances();
    TESTINFO( hosts.size() == 1, hosts.size( ));
    TESTINFO( service.get( hosts.front(), "foo" ) == "bar",
              service.get( hosts.front(), "foo" ));
    TEST( service.getKeys().size() == 2 );

    { // test updates during browsing
        lunchbox::Servus service2( serviceName.str( ));
        TEST( service2.announce( port+1,
                                 boost::lexical_cast< std::string >( port+1 )));
        TEST( service.browse( 2000 ));
        hosts = service.getInstances();
        TESTINFO( hosts.size() == 2, lunchbox::format( hosts ));
    }
    lunchbox::sleep( 500 );

    TEST( service.browse( 2000 ));
    hosts = service.getInstances();
    TESTINFO( hosts.size() == 1, lunchbox::format( hosts ));

    TEST( service.isBrowsing( ));
    service.endBrowsing();
    TEST( !service.isBrowsing( ));

    hosts = service.getInstances();
    TESTINFO( hosts.size() == 1, lunchbox::format( hosts ));
    TEST( service.get( hosts.front(), "foo" ) == "bar" );
    TEST( service.getKeys().size() == 2 );

    return EXIT_SUCCESS;
}
