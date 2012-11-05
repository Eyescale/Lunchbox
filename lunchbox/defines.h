
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_DEFINES_H
#define LUNCHBOX_DEFINES_H

#ifdef __APPLE__
#  include <lunchbox/definesDarwin.h>
#endif
#ifdef __linux
#  include <lunchbox/definesLinux.h>
#endif
#ifdef _WIN32 //_MSC_VER
#  include <lunchbox/definesWin32.h>
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef WIN32_API
#    define WIN32_API
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#endif

// Defining our own min/max macros seems to be the only sane way to get this
// functionality across platforms thanks to some screwup in the MS header files.
#define LB_MAX(a,b) ((a)>(b)?(a):(b)) //!< returns the maximum of two values
#define LB_MIN(a,b) ((a)<(b)?(a):(b)) //!< returns the minimum of two values

/** A 'NULL' value for an uint32.*/
#define LB_UNDEFINED_UINT32   (0xffffffffu)

/** The biggest usable value when using special uint32 values.*/
#define LB_MAX_UINT32         (0xfffffff0u)

/** Constant defining 'wait forever' in methods with wait parameters. */
#define LB_TIMEOUT_INDEFINITE 0xffffffffu // Attn: identical to Win32 INFINITE!

/** Constant defining use global default in methods with wait parameters. */
#define LB_TIMEOUT_DEFAULT 0xfffffffeu

#ifdef __GNUC__
#  define LB_LIKELY(x)       __builtin_expect( (x), 1 )
#  define LB_UNLIKELY(x)     __builtin_expect( (x), 0 )
#else
#  define LB_LIKELY(x)       x
#  define LB_UNLIKELY(x)     x
#endif

#endif // LUNCHBOX_DEFINES_H
