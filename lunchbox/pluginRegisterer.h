
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#ifndef LUNCHBOX_PLUGINREGISTERER_H
#define LUNCHBOX_PLUGINREGISTERER_H

#include <lunchbox/plugin.h> // used inline
#include <lunchbox/pluginFactory.h> // used inline

#include <boost/functional/factory.hpp>
#include <functional>

namespace lunchbox
{
/**
 * Helper class to statically register derived plugin classes.
 *
 * The following code can be placed in a plugin's cpp file:
 * @code
 * namespace
 * {
 *     PluginRegisterer< MyPlugin > registerer;
 * }
 * @endcode
 *
 * The plugin needs to conform to the following API:
 * @code
 * class MyPluginInterface
 * {
 * public:
 *     typedef MyPluginInterface InterfaceT;
 *     typedef MyPluginInitData InitDataT;
 * };
 *
 * class MyPlugin : public MyPluginInterface
 * {
 * public:
 *     MyPlugin( const InitDataT& data );
 *     static bool handles( const InitDataT& data );
 * };
 * @endcode
 *
 * @version 1.11.0
 */

template< typename T > class PluginRegisterer
{
public:
    /** Construct and register the Plugin< T > class. @version 1.11.0 */
    PluginRegisterer()
    {
        Plugin< typename T::InterfaceT > plugin(
            std::bind( boost::factory< T* >(), std::placeholders::_1 ),
            std::bind( &T::handles, std::placeholders::_1 ));
        PluginFactory< typename T::InterfaceT >::getInstance().register_(
            plugin );
    }
};
}

#endif
