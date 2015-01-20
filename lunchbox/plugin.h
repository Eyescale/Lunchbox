
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include <boost/function.hpp> // Plugin functions

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
 * They must also implement the following method to be registered:
 * @code
 * static bool handles( const InitDataT& initData );
 * @endcode
 *
 * @version 1.10.0
 */
template< class PluginT, class InitDataT = URI > class Plugin
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
     * @version 1.10.0
     */
    Plugin( const Constructor& constructor_, const HandlesFunc& handles_ )
        : constructor( constructor_ ), handles( handles_ ) {}

private:
    friend class PluginFactory< PluginT, InitDataT >;

    Constructor constructor;
    HandlesFunc handles;
};

}

#endif
