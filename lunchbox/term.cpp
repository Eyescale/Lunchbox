
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

#include "term.h"
#include "os.h"

#ifndef _MSC_VER
#  include <stdio.h>
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

#include <iostream>
namespace lunchbox
{
namespace term
{
size getSize()
{
#ifdef _MSC_VER
    CONSOLE_SCREEN_BUFFER_INFO info;
    if( GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &info ))
        return { info.dwSize.X, info.dwSize.Y };
#else
    struct winsize w;
    if( ::ioctl( STDOUT_FILENO, TIOCGWINSZ, &w ) >= 0 )
        return { w.ws_col, w.ws_row };
#endif
    return { 120, 80 };
}
}
}
