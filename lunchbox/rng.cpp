
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "rng.h"

#pragma warning(push)
#pragma warning(disable : 4985) // inconsistent decl of ceil

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <wtypes.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#else
#include <unistd.h>
#endif

#include <cstdlib>
#include <fcntl.h>
#include <limits>
#include <stdio.h>
#pragma warning(pop)

namespace lunchbox
{
namespace
{
#ifdef __linux__

static int _fd = -1;

void _exit()
{
    if (_fd >= 0)
    {
        ::close(_fd);
        _fd = -1;
    }
}

int _init()
{
    const int fd = ::open("/dev/urandom", O_RDONLY);
    if (fd >= 0)
        ::atexit(_exit);
    else
    {
        LBERROR << "Failed to open /dev/urandom: " << sysError << std::endl;
        return -1;
    }
    _fd = fd;
    return fd;
}

#elif defined _MSC_VER
static HCRYPTPROV _provider = 0;

void _exit()
{
    if (_provider && !CryptReleaseContext(_provider, 0))
        LBERROR << "Failed to release crypto context: " << sysError
                << std::endl;
    _provider = 0;
}

HCRYPTPROV _init()
{
    HCRYPTPROV provider = 0;
    if (CryptAcquireContext(&provider, 0, 0, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT) ||
        !provider)
    {
        ::atexit(_exit);
    }
    else
    {
        LBERROR << "Failed to acquire crypto context: " << sysError
                << std::endl;
        return 0;
    }

    _provider = provider;
    return provider;
}
#endif
}

RNG::RNG()
{
#ifdef __APPLE__
    srandomdev();
#endif
}

RNG::~RNG()
{
}

bool RNG::_get(void* data, const size_t size)
{
#ifdef __linux__
    static int fd = _init();
    int read = ::read(fd, data, size);
    LBASSERTINFO(read == ssize_t(size), read << " != " << size << ": "
                                             << sysError);
    if (read != ssize_t(size))
    {
        LBERROR << "random number generator not working" << std::endl;
        return false;
    }

#elif defined _MSC_VER
    static HCRYPTPROV provider = _init();
    if (!CryptGenRandom(provider, (DWORD)size, (BYTE*)data))
    {
        LBERROR << "random number generator not working" << std::endl;
        return false;
    }
#else // __APPLE__
    uint8_t* ptr = reinterpret_cast<uint8_t*>(data);
    for (size_t i = 0; i < size; ++i)
        ptr[i] = (random() & 0xff);
#endif
    return true;
}
}
