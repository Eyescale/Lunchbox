
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_FILE_H
#define LUNCHBOX_FILE_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h>
#include <lunchbox/types.h>

namespace lunchbox
{

/**
 * Retrieve a list of files in a directory matching a boost::regex pattern.
 *
 * @return all file names matching the given pattern in the given directory.
 * @version 1.0
 * @version 1.7.1 using boost::regex for matching
 * @deprecated Use Boost.Filesystem
 */
LUNCHBOX_API Strings searchDirectory( const std::string& directory,
                                      const std::string& pattern );
//    LB_DEPRECATED;

/**
 * @return the file name part of a path. @version 1.0
 * @deprecated Use Boost.Filesystem
 */
LUNCHBOX_API std::string getFilename( const std::string& filename );
//    LB_DEPRECATED;

/**
 * @return the directory name part of a path. @version 1.0
 * @deprecated Use Boost.Filesystem
 */
LUNCHBOX_API std::string getDirname( const std::string& filename )
    LB_DEPRECATED;

/**
 * @return the absolute path to the current executable.
 * @version 1.10
 */
LUNCHBOX_API std::string getExecutablePath();

/**
 * @return the absolute path to the libraries of the current executable.
 * @version 1.11
 */
LUNCHBOX_API std::string getLibraryPath();

/**
 * @return the search paths to libraries of the current executable.
 * @version 1.11
 */
LUNCHBOX_API Strings getLibraryPaths();
}

#endif //LUNCHBOX_FILE_H
