
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
#include <boost/function.hpp> // Plugin functions
#include <boost/function_equal.hpp> // operator ==

namespace lunchbox
{
/**
 * Manages a class deriving from a PluginT interface.
 *
 * Plugin classes deriving from PluginT must implement the following
 * prototype for their constructor:
 * @code
 * DerivedPluginClass( const InitDataT& initData );
 * @endcode
 *
 * PluginT must also implement the following method to be registered:
 * @code
 * static bool handles( const InitDataT& initData );
 * @endcode
 *
 * @version 1.11.0
 */
template< class PluginT, class InitDataT = servus::URI > class Plugin
{
public:
    /**
     * The constructor method / concrete factory for Plugin objects.
     * @version 1.11.0
     */
    typedef boost::function< PluginT* ( const InitDataT& ) > Constructor;

    /**
     * The method to check if the plugin can handle a given initData.
     * @version 1.11.0
     */
    typedef boost::function< bool ( const InitDataT& ) > HandlesFunc;

    /**
     * Construct a new Plugin.
     * @param constructor_ The constructor method for Plugin objects.
     * @param handles_ The method to check if the plugin can handle the
     * initData.
     * @version 1.11.0
     */
    Plugin( const Constructor& constructor, const HandlesFunc& handles )
        : _constructor( constructor ), _handles( handles )
        , tag( servus::make_UUID( )) {}

    /** @return true if the plugins wrap the same plugin. @version 1.11.0 */
    bool operator == ( const Plugin& rhs ) const
        { return tag == rhs.tag; }

    /** @return false if the plugins do wrap the same plugin. @version 1.11.0 */
    bool operator != ( const Plugin& rhs ) const { return !(*this == rhs); }

    /** Construct a new plugin instance. @version 1.14 */
    PluginT* construct( const InitDataT& data ) { return _constructor( data ); }

    /** @return true if this plugin handles the given request. @version 1.14 */
    bool handles( const InitDataT& data ) { return _handles( data ); }

private:
    Constructor _constructor;
    HandlesFunc _handles;

    // Makes Plugin comparable. See http://stackoverflow.com/questions/18665515
    servus::uint128_t tag;
};

}

#endif
