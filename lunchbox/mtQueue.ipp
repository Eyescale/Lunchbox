
/* Copyright (c) 2005-2017, Stefan Eilemann <eile@equalizergraphics.com>
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
template <typename T, size_t S>
MTQueue<T, S>& MTQueue<T, S>::operator=(const MTQueue<T, S>& from)
{
    if (this == &from)
        return *this;

    std::unique_lock<std::mutex> fromLock(from._mutex);
    std::deque<T> copy = from._queue;
    const size_t maxSize = from._maxSize;
    fromLock.unlock();

    std::unique_lock<std::mutex> lock(_mutex);
    _maxSize = maxSize;
    _queue.swap(copy);
    _condition.notify_all();
    return *this;
}

template <typename T, size_t S>
const T& MTQueue<T, S>::operator[](const size_t index) const
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return _queue.size() > index; });

    return _queue[index];
}

template <typename T, size_t S>
void MTQueue<T, S>::setMaxSize(const size_t maxSize)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return _queue.size() <= maxSize; });

    _maxSize = maxSize;
    _condition.notify_all();
}

template <typename T, size_t S>
size_t MTQueue<T, S>::waitSize(const size_t minSize) const
{
    LBASSERT(minSize <= _maxSize);
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return _queue.size() >= minSize; });
    return _queue.size();
}

template <typename T, size_t S>
void MTQueue<T, S>::clear()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.clear();
    _condition.notify_all();
}

template <typename T, size_t S>
T MTQueue<T, S>::pop()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return !_queue.empty(); });

    T element = _queue.front();
    _queue.pop_front();
    _condition.notify_all();
    return element;
}

template <typename T, size_t S>
bool MTQueue<T, S>::timedPop(const unsigned timeout, T& element)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait_for(lock, std::chrono::milliseconds(timeout),
                        [&] { return !_queue.empty(); });
    if (_queue.empty())
        return false;

    element = _queue.front();
    _queue.pop_front();
    _condition.notify_all();
    return true;
}

template <typename T, size_t S>
std::vector<T> MTQueue<T, S>::timedPopRange(const unsigned timeout,
                                            const size_t minimum,
                                            const size_t maximum)
{
    std::vector<T> result;

    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait_for(lock, std::chrono::milliseconds(timeout),
                        [&] { return _queue.size() >= minimum; });
    if (_queue.size() < minimum)
        return result;

    const size_t size = LB_MIN(maximum, _queue.size());

    result.reserve(size);
    result.insert(result.end(), _queue.begin(), _queue.begin() + size);
    _queue.erase(_queue.begin(), _queue.begin() + size);

    _condition.notify_all();
    return result;
}

template <typename T, size_t S>
bool MTQueue<T, S>::tryPop(T& result)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty())
        return false;

    result = _queue.front();
    _queue.pop_front();
    _condition.notify_all();
    return true;
}

template <typename T, size_t S>
void MTQueue<T, S>::tryPop(const size_t num, std::vector<T>& result)
{
    std::unique_lock<std::mutex> lock(_mutex);
    const size_t size = LB_MIN(num, _queue.size());
    if (size > 0)
    {
        result.reserve(result.size() + size);
        for (size_t i = 0; i < size; ++i)
        {
            result.push_back(_queue.front());
            _queue.pop_front();
        }
        _condition.notify_all();
    }
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

    std::unique_lock<std::mutex> lock(_mutex);
    ++barrier.waiting_;
    _condition.wait(lock, [&] {
        return !_queue.empty() || barrier.waiting_ >= barrier.height_;
    });

    if (_queue.empty())
    {
        LBASSERT(barrier.waiting_ == barrier.height_);
        _condition.notify_all();
        return false;
    }

    element = _queue.front();
    _queue.pop_front();
    --barrier.waiting_;
    _condition.notify_all();
    return true;
}

template <typename T, size_t S>
bool MTQueue<T, S>::getFront(T& result) const
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty())
        return false;

    // else
    result = _queue.front();
    return true;
}

template <typename T, size_t S>
bool MTQueue<T, S>::getBack(T& result) const
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty())
        return false;

    // else
    result = _queue.back();
    return true;
}

template <typename T, size_t S>
void MTQueue<T, S>::push(const T& element)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return _queue.size() < _maxSize; });
    _queue.push_back(element);
    _condition.notify_all();
}

template <typename T, size_t S>
void MTQueue<T, S>::push(const std::vector<T>& elements)
{
    std::unique_lock<std::mutex> lock(_mutex);
    LBASSERT(elements.size() <= _maxSize);
    _condition.wait(lock, [&] {
        return (_maxSize - _queue.size()) >= elements.size();
    });
    ;
    _queue.insert(_queue.end(), elements.begin(), elements.end());
    _condition.notify_all();
}

template <typename T, size_t S>
void MTQueue<T, S>::pushFront(const T& element)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&] { return _queue.size() < _maxSize; });
    ;
    _queue.push_front(element);
    _condition.notify_all();
}

template <typename T, size_t S>
void MTQueue<T, S>::pushFront(const std::vector<T>& elements)
{
    std::unique_lock<std::mutex> lock(_mutex);
    LBASSERT(elements.size() <= _maxSize);
    _condition.wait(lock, [&] {
        return (_maxSize - _queue.size()) >= elements.size();
    });
    ;
    _queue.insert(_queue.begin(), elements.begin(), elements.end());
    _condition.notify_all();
}
}
