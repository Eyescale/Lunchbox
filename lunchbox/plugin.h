
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *
 * This file is part of Lunchbox <https://github.com/Eyescale/Lunchbox>
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

#ifndef LUNCHBOX_PLUGIN_H
#define LUNCHBOX_PLUGIN_H

#include <servus/uint128_t.h> // member
#include <functional>

namespace lunchbox
{
/**
 * Manages a class deriving from a T interface.
 *
 * Plugin classes deriving from T must implement the following
 * prototype for their constructor:
 * @code
 * DerivedPluginClass( const T::InitDataT& initData );
 * @endcode
 *
 * T must also implement the following method to be registered:
 * @code
 * static bool handles( const T::InitDataT& initData );
 * @endcode
 *
 * Note this requires a 'typedef [foo] InitDataT' in T.
 * @version 1.11.0
 */
template< class T > class Plugin
{
public:
    /**
     * The constructor method / concrete factory for Plugin objects.
     * @version 1.11.0
     */
    typedef std::function< T* ( const typename T::InitDataT& )> Constructor;

    /**
     * The method to check if the plugin can handle a given initData.
     * @version 1.11.0
     */
    typedef std::function< bool ( const typename T::InitDataT& )> HandlesFunc;

    /**
     * Construct a new Plugin.
     * @param constructor_ The constructor method for Plugin objects.
     * @param handles_ The method to check if the plugin can handle the
     * initData.
     * @version 1.11.0
     */
    Plugin( const Constructor& constructor, const HandlesFunc& handles_ )
        : _constructor( constructor ), _handles( handles_ ) {}

    /** Construct a new plugin instance. @version 1.14 */
    T* construct( const typename T::InitDataT& data )
        { return _constructor( data ); }

    /** @return true if this plugin handles the given request. @version 1.14 */
    bool handles( const typename T::InitDataT& data )
        { return _handles( data ); }

private:
    Constructor _constructor;
    HandlesFunc _handles;

    bool operator == ( const Plugin& rhs ) const = delete;
    bool operator != ( const Plugin& rhs ) const = delete;
};

}

#endif
