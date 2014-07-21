
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
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
    {
        const std::string uriStr = "http://www.example.com/path/?key=value#fragment";
        lunchbox::URI uri( uriStr );

        TEST( uri.isValid( ) );
        TESTINFO( uri.getScheme() == "http", uri.getScheme() );
        TESTINFO( uri.getHost() == "www.example.com", uri.getHost()  );
        TESTINFO( uri.getPath() == "/path/", uri.getPath()  );
        TESTINFO( uri.getQuery() == "key=value", uri.getQuery()  );
        TESTINFO( uri.getFragment() == "fragment", uri.getFragment()  );

        std::stringstream sstr;
        sstr << uri;
        TESTINFO( sstr.str() == uriStr, sstr.str() << " " <<  uriStr );
    }

    {
        const std::string uriStr = "Helloworld";
        lunchbox::URI uri( uriStr );

        TEST( !uri.isValid( ) );
    }

    return EXIT_SUCCESS;
}





