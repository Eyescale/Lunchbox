
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

#ifndef LUNCHBOX_PLUGINREGISTERER_H
#define LUNCHBOX_PLUGINREGISTERER_H

#include <lunchbox/plugin.h> // used inline
#include <lunchbox/pluginFactory.h> // used inline

#include <boost/bind.hpp> // used inline
#include <boost/version.hpp>
#if BOOST_VERSION >= 104300
#  include <boost/functional/factory.hpp>
#else
#  include <lunchbox/factory.hpp>
#endif

namespace lunchbox
{
/**
 * Helper class to statically register derived plugin classes. If MyInitDataType
 * is not given, default value is lunchbox::URI.
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
 *     typedef MyPluginInterface PluginT;
 *     typedef MyPluginInitData InitDataT;
 *              ( optional for InitDataT == lunchbox::URI )
 * }
 * @endcode
 *
 * @version 1.11.0
 */
template< typename T > struct hasInitDataT
{
    // SFINAE class to check whether class T has a typedef InitDataT
    // If class has the typedef, "value" is known in compile time as true,
    // else value is false.

    // SFINAE is used for specializing the PluginRegisterer class
    // when no InitDataT is defined.
    template<typename U> static char (&test(typename U::InitDataT const*))[1];
    template<typename U> static char (&test(...))[2];
    // cppcheck-suppress sizeofCalculation
    static const bool value = (sizeof(test<T>(0)) == 1);
};

template< typename Impl, bool hasInitData = hasInitDataT< Impl >::value >
class PluginRegisterer
{
public:
    /** Construct a registerer and register the Impl class. @version 1.11.0 */
    PluginRegisterer();
};

/**
 * Specialized PluginRegisterer for implementations which have the InitDataT
 * definition.
 */
template< typename Impl > class PluginRegisterer< Impl, true >
{
public:
    /** Construct a registerer and register the Impl class. @version 1.11.0 */
    PluginRegisterer()
    {
        Plugin< typename Impl::PluginT, typename Impl::InitDataT > plugin(
            boost::bind( boost::factory< Impl* >(), _1 ),
            boost::bind( &Impl::handles, _1 ));
        PluginFactory< typename Impl::PluginT,
                       typename Impl::InitDataT >::getInstance().
            register_( plugin );
    }
};

/**
 * Specialized PluginRegisterer for plugin implementations which don't have
 * the InitDataT definition.
 */
template< typename Impl > class PluginRegisterer< Impl, false >
{
public:
    /** Construct a registerer and register the Impl class. @version 1.11.0 */
    PluginRegisterer()
    {
        Plugin< typename Impl::PluginT > plugin(
            boost::bind( boost::factory< Impl* >(), _1 ),
            boost::bind( &Impl::handles, _1 ));

        PluginFactory< typename Impl::PluginT >::getInstance().
            register_( plugin );
    }
};

}

#endif
