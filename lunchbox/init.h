
/* Copyright (c) 2008-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_INIT_H
#define LUNCHBOX_INIT_H

#include <lunchbox/api.h>

namespace lunchbox
{
/**
 * Initialize the Lunchbox base classes.
 *
 * exit() should be called independent of the return value of this function. The
 * following arguments are parsed:
 * * -v[v]: Increase log verbosity
 * * --lb-logfile [filename]: redirect log to the given file or to
 *   ApplicationName.log
 *
 * @param argc the command line argument count.
 * @param argv the command line argument values.
 * @return true if the library was successfully initialised, false otherwise
 * @version 1.0
 */
LUNCHBOX_API bool init(const int argc, char** argv);

/**
 * De-initialize the Lunchbox base classes.
 *
 * @return true if the library was successfully de-initialised, false otherwise
 * @version 1.0
 */
LUNCHBOX_API bool exit();
}
#endif // LUNCHBOX_INIT_H
