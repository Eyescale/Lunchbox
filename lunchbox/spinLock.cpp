
/* Copyright (c) 2012-2017, Stefan Eilemann <eile@eyescale.ch>
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

#include "spinLock.h"
#include <lunchbox/atomic.h>
#include <lunchbox/thread.h>

namespace lunchbox
{
namespace
{
static const long _writelocked = -1;
static const long _unlocked = 0;
}

class SpinLock::Impl
{
public:
    Impl()
        : _state(_unlocked)
    {
    }

    ~Impl() { _state = _unlocked; }

    inline void set()
    {
        while (true)
        {
            if (trySet())
                return;
            lunchbox::Thread::yield();
        }
    }

    inline void unset()
    {
        LBASSERT(_state == _writelocked);
        _state = _unlocked;
    }

    inline bool trySet()
    {
        if (!_state.compareAndSwap(_unlocked, _writelocked))
            return false;
        LBASSERTINFO(isSetWrite(), _state);
        return true;
    }

    inline void setRead()
    {
        while (true)
        {
            if (trySetRead())
                return;
            lunchbox::Thread::yield();
        }
    }

    inline void unsetRead()
    {
        while (true)
        {
            LBASSERT(_state > _unlocked);
            memoryBarrier();
            const int32_t expected = _state;
            if (_state.compareAndSwap(expected, expected - 1))
                return;
        }
    }

    inline bool trySetRead()
    {
        memoryBarrier();
        const int32_t state = _state;
        // Note: 0 used here since using _unlocked unexplicably gives
        //       'undefined reference to lunchbox::SpinLock::_unlocked'
        const int32_t expected = (state == _writelocked) ? 0 : state;

        if (!_state.compareAndSwap(expected, expected + 1))
            return false;

        LBASSERTINFO(isSetRead(), _state << ", " << expected);
        return true;
    }

    inline bool isSet() { return (_state != _unlocked); }
    inline bool isSetWrite() { return (_state == _writelocked); }
    inline bool isSetRead() { return (_state > _unlocked); }

private:
    a_int32_t _state;
};

SpinLock::SpinLock()
    : _impl(new SpinLock::Impl)
{
}

SpinLock::~SpinLock()
{
}

void SpinLock::set()
{
    _impl->set();
}

void SpinLock::unset()
{
    _impl->unset();
}

bool SpinLock::trySet()
{
    return _impl->trySet();
}

void SpinLock::setRead()
{
    _impl->setRead();
}

void SpinLock::unsetRead()
{
    _impl->unsetRead();
}

bool SpinLock::trySetRead()
{
    return _impl->trySetRead();
}

bool SpinLock::isSet()
{
    return _impl->isSet();
}

bool SpinLock::isSetWrite()
{
    return _impl->isSetWrite();
}

bool SpinLock::isSetRead()
{
    return _impl->isSetRead();
}
}
