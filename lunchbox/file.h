
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <lunchbox/types.h>

namespace lunchbox
{

/**
 * Retrieve a list of files in a directory matching a pattern.
 *
 * Only foo*bar pattern are implemented currently.
 *
 * @return all file names matching the given pattern in the given directory.
 * @version 1.0
 */
LUNCHBOX_API Strings searchDirectory( const std::string& directory,
                                    const std::string& pattern );

/** @return the file name part of a path. @version 1.0 */
LUNCHBOX_API std::string getFilename( const std::string& filename );

/** @return the directory name part of a path. @version 1.0 */
LUNCHBOX_API std::string getDirname( const std::string& filename );

}
#endif //LUNCHBOX_FILE_H
