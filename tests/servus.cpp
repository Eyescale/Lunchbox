
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
    lunchbox::Servus service( "_servustest._tcp" );

    const lunchbox::Servus::Result& result = service.announce( port,
                                    boost::lexical_cast< std::string >( port ));

#ifdef LUNCHBOX_USE_DNSSD
    TEST( lunchbox::Result::SUCCESS == kDNSServiceErr_NoError );
    if( result == kDNSServiceErr_Unknown ) // happens on CI VMs
    {
        std::cerr << "Bailing, got " << result
                  << ": looks like a broken zeroconf setup" << std::endl;
        return EXIT_SUCCESS;
    }
    TESTINFO( result, result );

    service.withdraw();
    service.set( "foo", "bar" );
    TEST( service.announce( port, boost::lexical_cast< std::string >( port )));

    const lunchbox::Strings& hosts =
        service.discover( lunchbox::Servus::IF_LOCAL, 200 );
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
    lunchbox::sleep( 200 );
    service.discover( lunchbox::Servus::IF_LOCAL, 200 );
    TEST( service.get( hosts.front(), "foobar" ) == "42" );
    TEST( service.getKeys().size() == 2 );

    // continuous browse API
    TEST( !service.isBrowsing( ));
    TEST( service.beginBrowsing( lunchbox::Servus::IF_LOCAL ));
    TEST( service.isBrowsing( ));
    TEST( service.beginBrowsing( lunchbox::Servus::IF_LOCAL ) ==
          kDNSServiceErr_AlreadyRegistered );
    TEST( service.isBrowsing( ));

    TESTINFO( service.browse( 200 ), service.browse( 0 ));
    TESTINFO( service.get( hosts.front(), "foobar" ) == "42",
              service.get( hosts.front(), "foobar" ));
    TEST( service.getKeys().size() == 2 );

    { // test updates during browsing
        lunchbox::Servus service2( "_servustest._tcp" );
        TEST( service2.announce( port+1,
                                 boost::lexical_cast< std::string >( port+1 )));
        TEST( service.browse( 200 ));
        TEST( service.getInstances().size() == 2 );
    }

    TEST( service.browse( 200 ));
    TESTINFO( service.getInstances().size() == 1,
              lunchbox::format( service.getInstances( )));

    TEST( service.isBrowsing( ));
    service.endBrowsing();
    TEST( !service.isBrowsing( ));
    TEST( service.get( hosts.front(), "foo" ) == "bar" );
    TEST( service.getKeys().size() == 2 );

#else
    TESTINFO( result == lunchbox::Servus::Result::NOT_SUPPORTED, result );
#endif
    return EXIT_SUCCESS;
}
