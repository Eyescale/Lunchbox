
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

namespace lunchbox
{
template <class T>
template <typename A, typename D>
Buffer<T>::Buffer(const A& alloc, const D& dealloc)
    : _data(nullptr)
    , _size(0)
    , _maxSize(0)
    , _alloc(alloc)
    , _dealloc(dealloc)
{
}

template <class T>
template <typename A, typename D>
Buffer<T>::Buffer(const uint64_t size, const A& alloc, const D& dealloc)
    : _alloc(alloc)
    , _dealloc(dealloc)
{
    reset(size);
}

template <class T>
Buffer<T>::Buffer(const Buffer<T>& from)
    : _data(nullptr)
    , _size(0)
    , _maxSize(0)
    , _alloc(from._alloc)
    , _dealloc(from._dealloc)
{
    if (!from.isEmpty())
        *this = from;
}

template <class T>
Buffer<T>::Buffer(Buffer<T>&& from)
    : _data(std::move(from._data))
    , _size(from._size)
    , _maxSize(from._maxSize)
    , _alloc(std::move(from._alloc))
    , _dealloc(std::move(from._dealloc))
{
    from._data = nullptr;
    from._size = 0;
    from._maxSize = 0;
}

template <class T>
void Buffer<T>::clear()
{
    if (_data)
    {
        if (_dealloc)
            _dealloc(_data);
        else
            free(_data);
    }
    _data = 0;
    _size = 0;
    _maxSize = 0;
}

template <class T>
T* Buffer<T>::pack()
{
    if (_maxSize != _size)
        _realloc(_size);

    return _data;
}

template <class T>
Buffer<T>& Buffer<T>::operator=(const Buffer<T>& from)
{
    if (this != &from)
        replace(from);
    return *this;
}

template <class T>
Buffer<T>& Buffer<T>::operator=(Buffer<T>&& from)
{
    if (this == &from)
        return *this;
    std::swap(_data, from._data);
    std::swap(_size, from._size);
    std::swap(_maxSize, from._maxSize);
    std::swap(_alloc, from._alloc);
    std::swap(_dealloc, from._dealloc);
    return *this;
}

template <class T>
T* Buffer<T>::resize(const uint64_t newSize)
{
    _size = newSize;
    if (newSize <= _maxSize)
        return _data;

    // avoid excessive reallocs
    const uint64_t nElems = newSize + (newSize >> 3);
    _realloc(nElems);
    return _data;
}

template <class T>
void Buffer<T>::grow(const uint64_t newSize)
{
    if (newSize > _size)
        resize(newSize);
}

template <class T>
T* Buffer<T>::reserve(const uint64_t newSize)
{
    if (newSize <= _maxSize)
        return _data;

    _realloc(newSize);
    return _data;
}

template <class T>
T* Buffer<T>::reset(const uint64_t newSize)
{
    reserve(newSize);
    setSize(newSize);
    return _data;
}

template <class T>
void Buffer<T>::append(const T* data, const uint64_t size)
{
    LBASSERT(data);
    LBASSERT(size);

    const uint64_t oldSize = _size;
    resize(oldSize + size);
    memcpy(_data + oldSize, data, size * sizeof(T));
}

template <class T>
void Buffer<T>::append(const T& element)
{
    append(&element, 1);
}

template <class T>
void Buffer<T>::replace(const void* data, const uint64_t size)
{
    LBASSERT(data);
    LBASSERT(size);

    // Possible optimization, do not use reserve as it performs a useless
    // memcpy if it needs to realloc.
    reserve(size);
    memcpy(_data, data, size * sizeof(T));
    _size = size;
}

template <class T>
void Buffer<T>::replace(const Buffer& from)
{
    replace(from._data, from._size);
    _alloc = from._alloc;
    _dealloc = from._dealloc;
}

template <class T>
void Buffer<T>::swap(Buffer<T>& buffer)
{
    std::swap(_data, buffer._data);
    std::swap(_size, buffer._size);
    std::swap(_maxSize, buffer._maxSize);
    std::swap(_alloc, buffer._alloc);
    std::swap(_dealloc, buffer._dealloc);
}

template <class T>
bool Buffer<T>::setSize(const uint64_t size)
{
    LBASSERT(size <= _maxSize);
    if (size > _maxSize)
        return false;

    _size = size;
    return true;
}

template <class T>
void Buffer<T>::_realloc(const uint64_t size)
{
    if (_alloc)
    {
        T* data = _alloc(size * sizeof(T));
        if (_data)
        {
            memcpy(data, _data, std::min(_size, size) * sizeof(T));
            _dealloc(_data);
        }
        _data = data;
    }
    else
    {
        _data = static_cast<T*>(realloc(_data, size * sizeof(T)));
    }
    _maxSize = size;
}
}
