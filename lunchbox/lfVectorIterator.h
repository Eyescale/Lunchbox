
/* Copyright (c) 2011-2013, EPFL/Blue Brain Project
 *                          Stefan Eilemann <stefan.eilemann@epfl.ch>
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#ifndef LUNCHBOX_LFVECTOR_ITERATOR_H
#define LUNCHBOX_LFVECTOR_ITERATOR_H

#include <lunchbox/indexIterator.h>

namespace lunchbox
{
/**
 * An iterator for LFVector.
 *
 * @tparam V the LFVector type
 * @tparam T the LFVector element type
 */
template <class V, class T>
class LFVectorIterator : public IndexIterator<LFVectorIterator<V, T>, V, T>
{
    typedef IndexIterator<LFVectorIterator<V, T>, V, T> Super;

public:
    /** Construct an iterator for a given vector and position. @version 1.8 */
    LFVectorIterator(V* vector, size_t i)
        : Super(vector, i)
    {
    }

    /** Copy-construct an iterator.  @version 1.8 */
    template <class U, class W>
    LFVectorIterator(const LFVectorIterator<U, W>& from)
        : Super(from)
    {
    }

    /** Dereference the element at the current position. @version 1.8 */
    T& operator*() const { return (*Super::container_)[Super::i_]; }
    /** Address the element at the current position. @version 1.8 */
    T* operator->() const { return &(*Super::container_)[Super::i_]; }
    /** Address the element at the given position. @version 1.8 */
    T& operator[](const size_t& n) const
    {
        return (*Super::container_)[Super::i_ + n];
    }

private:
    template <class, int32_t>
    friend class LFVector; // LFVector::erase
};
}

#endif // LUNCHBOX_LFVECTOR_ITERATOR_H
