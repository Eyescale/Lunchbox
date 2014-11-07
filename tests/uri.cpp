
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
        lunchbox::URI uri( uriStr );

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

        uri.addQuery( "hans", "dampf" );
        TESTINFO( uri.findQuery( "key" )->second == "value",
                  uri.findQuery( "key" )->second );
        TESTINFO( uri.findQuery( "foo" )->second == "bar",
                  uri.findQuery( "foo" )->second );
        TESTINFO( uri.findQuery( "hans" )->second == "dampf",
                  uri.findQuery( "hans" )->second );
        TESTINFO( uri.getQuery().find( "hans=dampf" ) != std::string::npos,
                  uri.getQuery( ));

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

        lunchbox::URI empty;
        TEST( empty.getScheme().empty( ));
        TEST( empty.getHost().empty( ));
        TEST( empty.getUserinfo().empty( ));
        TEST( empty.getPort() == 0 );
        TEST( empty.getPath().empty( ));
        TEST( empty.getQuery().empty( ));
        TEST( empty.getFragment().empty( ));

        lunchbox::URI file1( "/bla.txt" );
        TEST( file1.getPath() == "/bla.txt" );
        TEST( file1.getHost().empty( ));
        TEST( file1.getScheme().empty( ));

        lunchbox::URI file2( "bla.txt" );
        TEST( file2.getPath() == "bla.txt" );
        TEST( file2.getHost().empty( ));
        TEST( file2.getScheme().empty( ));

        lunchbox::URI file3( "file:///bla.txt" );
        TEST( file3.getPath() == "/bla.txt" );
        TEST( file3.getHost().empty( ));
        TEST( file3.getScheme() == "file" );

        lunchbox::URI file4( "file://bla.txt" );
        TEST( file4.getPath() == "bla.txt" );
        TEST( file4.getHost().empty( ));
        TEST( file4.getScheme() == "file" );

        lunchbox::URI file5( "host://bla.txt" );
        TEST( file5.getHost() == "bla.txt" );
        TEST( file5.getPath().empty( ));
        TEST( file5.getScheme() == "host" );

        file5.setScheme( "foo" );
        TEST( file5.getScheme() == "foo" );
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
