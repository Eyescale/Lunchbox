
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
template <typename T, size_t S>
MTQueue<T, S>& MTQueue<T, S>::operator=(const MTQueue<T, S>& from)
{
    if (this != &from)
    {
        from._cond.lock();
        std::deque<T> copy = from._queue;
        const size_t maxSize = from._maxSize;
        from._cond.unlock();

        _cond.lock();
        _maxSize = maxSize;
        _queue.swap(copy);
        _cond.signal();
        _cond.unlock();
    }
    return *this;
}

template <typename T, size_t S>
const T& MTQueue<T, S>::operator[](const size_t index) const
{
    _cond.lock();
    while (_queue.size() <= index)
        _cond.wait();

    LBASSERT(_queue.size() > index);
    const T& element = _queue[index];
    _cond.unlock();
    return element;
}

template <typename T, size_t S>
void MTQueue<T, S>::setMaxSize(const size_t maxSize)
{
    _cond.lock();
    while (_queue.size() > maxSize)
        _cond.wait();
    _maxSize = maxSize;
    _cond.signal();
    _cond.unlock();
}

template <typename T, size_t S>
size_t MTQueue<T, S>::waitSize(const size_t minSize) const
{
    LBASSERT(minSize <= _maxSize);
    _cond.lock();
    while (_queue.size() < minSize)
        _cond.wait();
    const size_t size = _queue.size();
    _cond.unlock();
    return size;
}

template <typename T, size_t S>
void MTQueue<T, S>::clear()
{
    _cond.lock();
    _queue.clear();
    _cond.signal();
    _cond.unlock();
}

template <typename T, size_t S>
T MTQueue<T, S>::pop()
{
    _cond.lock();
    while (_queue.empty())
        _cond.wait();

    LBASSERT(!_queue.empty());
    T element = _queue.front();
    _queue.pop_front();
    _cond.signal();
    _cond.unlock();
    return element;
}

template <typename T, size_t S>
bool MTQueue<T, S>::timedPop(const unsigned timeout, T& element)
{
    _cond.lock();
    while (_queue.empty())
    {
        if (!_cond.timedWait(timeout))
        {
            _cond.unlock();
            return false;
        }
    }
    LBASSERT(!_queue.empty());
    element = _queue.front();
    _queue.pop_front();
    _cond.signal();
    _cond.unlock();
    return true;
}

template <typename T, size_t S>
std::vector<T> MTQueue<T, S>::timedPopRange(const unsigned timeout,
                                            const size_t minimum,
                                            const size_t maximum)
{
    std::vector<T> result;

    _cond.lock();
    while (_queue.size() < minimum)
    {
        if (!_cond.timedWait(timeout))
        {
            _cond.unlock();
            return result;
        }
    }

    const size_t size = LB_MIN(maximum, _queue.size());

    result.reserve(size);
    result.insert(result.end(), _queue.begin(), _queue.begin() + size);
    _queue.erase(_queue.begin(), _queue.begin() + size);

    _cond.unlock();
    return result;
}

template <typename T, size_t S>
bool MTQueue<T, S>::tryPop(T& result)
{
    _cond.lock();
    if (_queue.empty())
    {
        _cond.unlock();
        return false;
    }

    result = _queue.front();
    _queue.pop_front();
    _cond.signal();
    _cond.unlock();
    return true;
}

template <typename T, size_t S>
void MTQueue<T, S>::tryPop(const size_t num, std::vector<T>& result)
{
    _cond.lock();
    const size_t size = LB_MIN(num, _queue.size());
    if (size > 0)
    {
        result.reserve(result.size() + size);
        for (size_t i = 0; i < size; ++i)
        {
            result.push_back(_queue.front());
            _queue.pop_front();
        }
        _cond.signal();
    }
    _cond.unlock();
}

/** Group descriptor for popBarrier(). @version 1.7.1 */
template <typename T, size_t S>
class MTQueue<T, S>::Group
{
    friend class MTQueue<T, S>;
    size_t height_;
    size_t waiting_;

public:
    /**
     * Construct a new group of the given size. Can only be used once.
     * @version 1.7.1
     */
    explicit Group(const size_t height)
        : height_(height)
        , waiting_(0)
    {
    }

    /** Update the height. @version 1.7.1  */
    void setHeight(const size_t height) { height_ = height; }
};

template <typename T, size_t S>
bool MTQueue<T, S>::popBarrier(T& element, Group& barrier)
{
    LBASSERT(barrier.height_ > 0)

    _cond.lock();
    ++barrier.waiting_;
    while (_queue.empty() && barrier.waiting_ < barrier.height_)
        _cond.wait();

    if (_queue.empty())
    {
        LBASSERT(barrier.waiting_ == barrier.height_);
        _cond.broadcast();
        _cond.unlock();
        return false;
    }

    element = _queue.front();
    _queue.pop_front();
    --barrier.waiting_;
    _cond.signal();
    _cond.unlock();
    return true;
}

template <typename T, size_t S>
bool MTQueue<T, S>::getFront(T& result) const
{
    _cond.lock();
    if (_queue.empty())
    {
        _cond.unlock();
        return false;
    }
    // else
    result = _queue.front();
    _cond.unlock();
    return true;
}

template <typename T, size_t S>
bool MTQueue<T, S>::getBack(T& result) const
{
    _cond.lock();
    if (_queue.empty())
    {
        _cond.unlock();
        return false;
    }
    // else
    result = _queue.back();
    _cond.unlock();
    return true;
}

template <typename T, size_t S>
void MTQueue<T, S>::push(const T& element)
{
    _cond.lock();
    while (_queue.size() >= _maxSize)
        _cond.wait();
    _queue.push_back(element);
    _cond.signal();
    _cond.unlock();
}

template <typename T, size_t S>
void MTQueue<T, S>::push(const std::vector<T>& elements)
{
    _cond.lock();
    LBASSERT(elements.size() <= _maxSize);
    while ((_maxSize - _queue.size()) < elements.size())
        _cond.wait();
    _queue.insert(_queue.end(), elements.begin(), elements.end());
    _cond.signal();
    _cond.unlock();
}

template <typename T, size_t S>
void MTQueue<T, S>::pushFront(const T& element)
{
    _cond.lock();
    while (_queue.size() >= _maxSize)
        _cond.wait();
    _queue.push_front(element);
    _cond.signal();
    _cond.unlock();
}

template <typename T, size_t S>
void MTQueue<T, S>::pushFront(const std::vector<T>& elements)
{
    _cond.lock();
    LBASSERT(elements.size() <= _maxSize);
    while ((_maxSize - _queue.size()) < elements.size())
        _cond.wait();
    _queue.insert(_queue.begin(), elements.begin(), elements.end());
    _cond.signal();
    _cond.unlock();
}
}
