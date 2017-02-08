
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

namespace lunchbox
{
template <typename T>
void LFQueue<T>::clear()
{
    LB_TS_SCOPED(_reader);
    _readPos = 0;
    _writePos = 0;
}

template <typename T>
void LFQueue<T>::resize(const int32_t size)
{
    LBASSERT(isEmpty());
    _readPos = 0;
    _writePos = 0;
    _data.resize(size + 1);
}

template <typename T>
bool LFQueue<T>::pop(T& result)
{
    LB_TS_SCOPED(_reader);
    if (_readPos == _writePos)
        return false;

    result = _data[_readPos];
    _readPos = (_readPos + 1) % _data.size();
    return true;
}

template <typename T>
bool LFQueue<T>::getFront(T& result)
{
    LB_TS_SCOPED(_reader);
    if (_readPos == _writePos)
        return false;

    result = _data[_readPos];
    return true;
}

template <typename T>
bool LFQueue<T>::push(const T& element)
{
    LB_TS_SCOPED(_writer);
    int32_t nextPos = (_writePos + 1) % _data.size();
    if (nextPos == _readPos)
        return false;

    _data[_writePos] = element;
    _writePos = nextPos;
    return true;
}
}
