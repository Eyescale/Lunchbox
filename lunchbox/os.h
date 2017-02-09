
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

/** Includes key system header files and defines essential base macros. */
#ifndef LUNCHBOX_OS_H
#define LUNCHBOX_OS_H

#include <lunchbox/api.h>
#include <lunchbox/compiler.h>
#include <lunchbox/defines.h>

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#ifndef WIN32_API
#define WIN32_API
#endif
#ifndef _MSC_VER
#define USE_SYS_TYPES_FD_SET
#endif
#ifndef _WIN32_WINNT       // Hopefully to higher than 0x500...
#define _WIN32_WINNT 0x501 // => XP, for WM_XBUTTONDOWN and others
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windef.h>
#include <windows.h>
#include <winsock2.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#ifndef _MSC_VER
#include <stdint.h>
#include <strings.h>
#include <sys/param.h> // for MIN/MAX
#endif

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#elif !defined(_WIN32)
extern "C" char** environ;
#endif

namespace lunchbox
{
/** OS-independent call to bzero(3). @version 1.7.1 */
static inline void setZero(void* ptr, const size_t size)
{
#ifdef _WIN32
    ::memset(ptr, 0, size);
#else
    ::bzero(ptr, size);
#endif
}

/** @return the local hostname. @version 1.9.2 */
LUNCHBOX_API std::string getHostname();
}

#endif // LUNCHBOX_OS_H
