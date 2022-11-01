
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "sleep.h"

#include "os.h"
#include "time.h"
#include <time.h>

namespace lunchbox
{
void sleep(const uint32_t milliSeconds)
{
#ifdef _WIN32 //_MSC_VER
    ::Sleep(milliSeconds);
#else
    timespec ts = convertToTimespec(milliSeconds);
    while (::nanosleep(&ts, &ts) != 0) // -1 on signal (#4)
        if (ts.tv_sec <= 0 && ts.tv_nsec <= 0)
            return;
#endif
}
}
