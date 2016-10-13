
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

#ifndef LUNCHBOX_PLUGINREGISTERER_H
#define LUNCHBOX_PLUGINREGISTERER_H

#include <lunchbox/plugin.h> // used inline
#include <lunchbox/pluginFactory.h> // used inline

#include <boost/bind.hpp> // used inline
#include <boost/version.hpp>
#include <boost/functional/factory.hpp>

namespace lunchbox
{
/**
 * Helper class to statically register derived plugin classes. If MyInitDataType
 * is not given, default value is servus::URI.
 *
 * The following code can be placed in a plugin's cpp file:
 * @code
 * namespace
 * {
 *     PluginRegisterer< MyPluginInterface > registerer;
 * }
 * @endcode
 *
 * Also note that it needs the following type definition to be placed in the
 * plugin's interface (or in all its implementations that are to be registered):
 * @code
 * class MyPluginInterface
 * {
 * public:
 *     typedef MyPluginInterface InterfaceT;
 *     typedef MyPluginInitData InitDataT;
 *              ( optional for InitDataT == servus::URI )
 * }
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
            boost::bind( boost::factory< T* >(), _1 ),
            boost::bind( &T::handles, _1 ));
        PluginFactory< typename T::InterfaceT >::getInstance().register_(
            plugin );
    }
};
}

#endif
