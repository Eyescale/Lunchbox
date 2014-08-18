
/* Copyright (c) 2013-2014, ahmet.bilgili@epfl.ch
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

#include <test.h>
#include <lunchbox/uri.h>

int main( int, char ** )
{
    try
    {
        const std::string uriStr =
            "http://bob@www.example.com:8080/path/?key=value,foo=bar#fragment";
        const lunchbox::URI uri( uriStr );

        TESTINFO( uri.getScheme() == "http", uri.getScheme() );
        TESTINFO( uri.getHost() == "www.example.com", uri.getHost( ));
        TESTINFO( uri.getUserinfo() == "bob", uri.getUserinfo( ));
        TESTINFO( uri.getPort() == 8080, uri.getPort( ));
        TESTINFO( uri.getPath() == "/path/", uri.getPath( ));
        TESTINFO( uri.getQuery() == "key=value,foo=bar", uri.getQuery( ));
        TESTINFO( uri.getFragment() == "fragment", uri.getFragment( ));

        TEST( uri.findQuery( "key" ) != uri.queryEnd( ));
        TEST( uri.findQuery( "foo" ) != uri.queryEnd( ));
        TEST( uri.findQuery( "bar" ) == uri.queryEnd( ));
        TESTINFO( uri.findQuery( "key" )->second == "value",
                  uri.findQuery( "key" )->second );
        TESTINFO( uri.findQuery( "foo" )->second == "bar",
                  uri.findQuery( "foo" )->second );

        std::stringstream sstr;
        sstr << uri;
        TESTINFO( sstr.str() == uriStr, sstr.str() << " " <<  uriStr );

        sstr.str( "" );
        sstr << lunchbox::URI( "http://www.example.com/path" );
        TESTINFO( sstr.str() == "http://www.example.com/path", sstr.str( ));

        sstr.str( "" );
        sstr << lunchbox::URI( "/path" );
        TESTINFO( sstr.str() == "/path", sstr.str( ));

        const lunchbox::URI hostPortURI( "foo://hostname:12345" );
        TESTINFO( hostPortURI.getScheme() == "foo", uri.getScheme() );
        TESTINFO( hostPortURI.getHost() == "hostname", uri.getHost( ));
        TESTINFO( hostPortURI.getPort() == 12345, uri.getPort( ));
    }
    catch( std::exception& exception )
    {
        LBERROR << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        const std::string uriStr = "Helloworld#?#://##";
        lunchbox::URI uri( uriStr );
        TESTINFO( false, "Did not get exception on malformed URI: " << uri );
    }
    catch( std::exception& ) {}

    return EXIT_SUCCESS;
}
