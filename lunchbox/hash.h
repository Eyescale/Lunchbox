
/* Copyright (c) 2005-2017, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef LUNCHBOX_HASH_H
#define LUNCHBOX_HASH_H

#include <lunchbox/refPtr.h>

#include <unordered_map>

namespace lunchbox
{
/** A hash for pointer keys. @version 1.0 */
template <class K, class T>
class PtrHash : public std::unordered_map<K, T, std::hash<const void*> >
{
};

template <typename T>
struct hashRefPtr
{
    size_t operator()(RefPtr<T> key) const
    {
        return std::hash<const void*>()(key.get());
    }
};

/** A hash for RefPtr keys. @version 1.0 */
template <class K, class T>
class RefPtrHash : public std::unordered_map<RefPtr<K>, T, hashRefPtr<K> >
{
};
}
#endif // LUNCHBOX_HASH_H
