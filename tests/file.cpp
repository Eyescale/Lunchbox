
/* Copyright (c) 2014, Daniel.Nachbaur@epfl.ch
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

#include "test.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <lunchbox/file.h>

int main( int, char** argv )
{
    const std::string& execPath = lunchbox::getExecutablePath();
    const boost::filesystem::path path( argv[0] );
    const std::string argvPath( path.parent_path().string( ));
    TEST( boost::algorithm::ends_with( execPath, argvPath ));
    return EXIT_SUCCESS;
}
