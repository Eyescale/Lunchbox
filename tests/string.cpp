
/* Copyright (c) 2017, Stefan.Eilemann@epfl.ch
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

#include <lunchbox/test.h>
#include <lunchbox/string.h>

int main( int, char** )
{
    TEST( lunchbox::string::prepend( "", "  " ) == "  " );
    TEST( lunchbox::string::prepend( "foo", " " ) == " foo" );
    TEST( lunchbox::string::prepend( "foo\nbar", " " ) == " foo\n bar" );
    TEST( lunchbox::string::prepend( "\nfoo\nbar", " " ) == " \n foo\n bar" );
    TEST( lunchbox::string::prepend( "\nfoo\nbar", "" ) == "\nfoo\nbar" );
    TEST( lunchbox::string::prepend( "\nfoo\nbar", "deine mutter " ) ==
                           "deine mutter \ndeine mutter foo\ndeine mutter bar" );
    return EXIT_SUCCESS;
}
