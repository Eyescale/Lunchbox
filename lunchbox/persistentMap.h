
/* Copyright (c) 2014, Stefan.Eilemann@epfl.ch
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

#ifndef LUNCHBOX_PERSISTENTMAP_H
#define LUNCHBOX_PERSISTENTMAP_H

#include <lunchbox/api.h>
#include <boost/noncopyable.hpp>
#include <iostream>
#include <string>

namespace lunchbox
{
namespace detail { class PersistentMap; }

/**
 * Unified interface to save key-value pairs in a persistent store.
 *
 * Available implementations:
 * * If LUNCHBOX_USE_LEVELDB is defined: leveldb://path
 *
 * Example: @include tests/persistentMap.cpp
 */
class PersistentMap : public boost::noncopyable
{
public:
    /**
     * Construct a new persistent map.
     *
     * Depending on the URI scheme an implementation backend is chosen. If no
     * URI is given, a default one is selected.
     *
     * @param uri the storage backend and destination.
     * @throw std::runtime_error if no suitable implementation is found.
     * @throw leveldb::Status if opening the leveldb database failed.
     * @version 1.9.2
     */
    LUNCHBOX_API PersistentMap( const std::string& uri = std::string( ));

    /** Destruct the persistent map. @version 1.9.2 */
    LUNCHBOX_API ~PersistentMap();

    /**
     * Insert or update a value in the database.
     *
     * @param key the key to store the value.
     * @param value the value stored at the key.
     * @return true on success, false otherwise
     * @version 1.9.2
     */
    LUNCHBOX_API bool insert( const std::string& key, const std::string& value);

    /**
     * Retrieve a value for a key.
     *
     * @param key the key to retreive.
     * @return the value, or an empty string if the key is not available.
     * @version 1.9.2
     */
    LUNCHBOX_API std::string operator [] ( const std::string& key ) const;

private:
    detail::PersistentMap* const _impl;
};

// inline std::ostream& operator << ( std::ostream& os, const PersistentMap& m )

}

#endif //LUNCHBOX_PERSISTENTMAP_H
