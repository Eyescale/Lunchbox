
/* Copyright (c) 2009-2015, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
 */
LUNCHBOX_API Strings searchDirectory(const std::string& directory,
                                     const std::string& pattern);

/**
 * @return the file name part of a path. @version 1.0
 */
LUNCHBOX_API std::string getFilename(const std::string& filename);

/**
 * @return the directory name part of a path. @version 1.0
 */
LUNCHBOX_API std::string getDirname(const std::string& filename);

/**
 * Get the absolute directory of the current executable.
 *
 * On Mac OS X, this returns the path to the app bundle, i.e., the directory
 * where the Foo.app is located, not Foo.app/Contents/MacOS.
 *
 * @return the absolute directory of the current executable.
 * @version 1.11
 */
LUNCHBOX_API std::string getExecutableDir();
/** @deprecated */
inline std::string getExecutablePath()
{
    return getExecutableDir();
}

/**
 * @return the absolute directory of the current working directory
 * @version 1.14
 */
LUNCHBOX_API std::string getWorkDir();

/**
 * Get the absolute path to the root directory of the current executable.
 *
 * On all platforms, this returns the root directory of the
 * installation/distribution of the current executable. Can be empty, if
 * getExecutableDir() is empty.
 *
 * On Linux and Mac OS X, this returns the directory one level up of
 * getExecutableDir().
 * On Windows, this returns the directory one or two levels up of
 * getExecutableDir(), depending if ${BuildType} is in the path.
 *
 * @return the absolute root directory of the current executable.
 * @version 1.12
 */
LUNCHBOX_API std::string getRootDir();
inline std::string getRootPath()
{
    return getRootDir();
} //!< @deprecated

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

/** Save binary to file. @return true on success, false otherwise. */
LUNCHBOX_API bool saveBinary(const servus::Serializable& object,
                             const std::string& filename);

/** Load from binary file. @return true on success, false otherwise. */
LUNCHBOX_API bool loadBinary(servus::Serializable& object,
                             const std::string& filename);

/** Save ascii (JSON) to file. @return true on success, false otherwise. */
LUNCHBOX_API bool saveAscii(const servus::Serializable& object,
                            const std::string& filename);

/** Load from ascii (JSON) file. @return true on success, false otherwise.*/
LUNCHBOX_API bool loadAscii(servus::Serializable& object,
                            const std::string& filename);
}

#endif // LUNCHBOX_FILE_H
