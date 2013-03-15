
/* Copyright (c) 2012, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_DAEMON_H
#define LUNCHBOX_DAEMON_H

#include <lunchbox/debug.h>
#ifndef _MSC_VER
#  include <unistd.h>
#endif

namespace lunchbox
{

/**
 * Turn the calling process into a daemon.
 *
 * Only the forked child process returns from this function. A new session is
 * created and the standard file descriptors are closed. The current working
 * directory is unchanged, and the Log output is not redirected to a file.
 *
 * @return true on success, false on error.
 * @version 1.5.1
 */
inline bool daemonize()
{
#ifdef _MSC_VER
    LBABORT( "Not implemented." );
    return false;
#else
    const pid_t pid = fork();
    if( pid < 0 )
    {
        LBWARN << "Fork failed: " << sysError << std::endl;
        return false;
    }

    if( pid > 0 ) // parent
        ::exit( EXIT_SUCCESS );

    const pid_t sid = setsid(); // child, create new seesion
    if( sid < 0 )
    {
        LBWARN << "setsid failed: " << sysError << std::endl;
        return false;
    }

    // Close the standard file
    ::close( STDIN_FILENO );
    ::close( STDOUT_FILENO );
    ::close( STDERR_FILENO );
    return true;
#endif
}
}

#endif //LUNCHBOX_DAEMON_H
