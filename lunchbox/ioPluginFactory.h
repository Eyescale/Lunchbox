
/* Copyright (c) 2013-2014, EPFL/Blue Brain Project
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#ifndef LUNCHBOX_IOPLUGINFACTORY_H
#define LUNCHBOX_IOPLUGINFACTORY_H

#include <lunchbox/types.h>

#include <lunchbox/lunchbox.h>
#include <lunchbox/uri.h>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 104300
#  include <boost/functional/factory.hpp>
#else
#  include <lunchbox/factory.hpp>
#endif

#include <map>
#include <utility>

namespace lunchbox
{

template< typename IOPluginT > class IOPlugin;

/**
 * Abstract factory for IOPluginT classes with plugin registration
 * functionality.
 *
 * The IOPluginAbstractFactory selects the most appropriate plugin factory for a
 * given URI based on a plugin's handles( URI ) function. In case a URI can be
 * handled by multiple plugins, which plugin is chosen is undefined.
 *
 * This class has been designed as a singleton to allow for link time plugin
 * registration, but nothing prevents an application from registering new types
 * at run time.
 *
 * To do the registration of a plugin during the static initialization phase
 * the class IOPluginRegisterer is provided.
 *
 * Example: @include tests/ioPluginFactory.cpp
 *
 * @version 1.10.0
 */
template< typename IOPluginT >
class IOPluginAbstractFactory : public boost::noncopyable
{
public:
    typedef std::vector< IOPlugin< IOPluginT > > IOPlugins;

    /** Get the single class instance. @version 1.10.0 */
    static IOPluginAbstractFactory& getInstance();

    /**
     * Create a plugin instance.
     * @param uri The uri passed to the plugin constructor.
     * @return A new IOPluginT instance. The user is responsible for deleting
     *         the returned object.
     * @throws std::runtime_error if no plugin can handle the uri.
     * @version 1.10.0
     */
    IOPluginT* create( const URI& uri );

    /** Register a plugin type. @version 1.10.0 */
    void registerPlugin( const IOPlugin< IOPluginT >& plugin );

    /** Unregister all plugin types. @version 1.10.0 */
    void unregisterAllPlugins();

private:
    IOPluginAbstractFactory() {}

    IOPlugins _plugins;
};

/**
 * Holds a concrete factory for a class deriving from a IOPluginT interface.
 *
 * Plugin classes deriving from IOPluginT must implement the following
 * prototype for their constructor:
 * @code
 * DerivedPluginClass( const URI& uri );
 * @endcode
 *
 * They must also implement the following method to be registered:
 * @code
 * static bool handles( const URI& uri );
 * @endcode
 *
 * @version 1.10.0
 */
template< typename IOPluginT >
class IOPlugin
{
public:
    /**
     * The constructor method / concrete factory for IOPlugin objects.
     * @version 1.10.0
     */
    typedef boost::function< IOPluginT* ( const URI& ) > Constructor;

    /**
     * The method to check if the plugin can handle a given uri.
     * @version 1.10.0
     */
    typedef boost::function< bool ( const URI& ) > HandlesFunc;

    /**
     * Construct a new IOPlugin.
     * @param constructor_ The constructor method for IOPlugin objects.
     * @param handles_ The method to check if the plugin can handle the uri.
     * @version 1.10.0
     */
    IOPlugin( const Constructor& constructor_, const HandlesFunc& handles_ );

private:
    friend class IOPluginAbstractFactory< IOPluginT >;

    Constructor constructor;
    HandlesFunc handles;
};

/**
 * Helper class to statically register derived plugin classes.
 *
 * The following code can be placed in a plugin's cpp file:
 * @code
 * namespace
 * {
 *     IOPluginRegisterer< MyPluginClass > registerer;
 * }
 * @endcode
 *
 * Also note that it needs the following type definition to be placed in the
 * plugin's interface (or in all its implementations that are to be registered):
 * @code
 * class MyPluginInterface
 * {
 * public:
 *     typedef MyPluginInterface IOPluginT;
 * }
 * @endcode
 *
 * @version 1.10.0
 */
template< typename Impl >
class IOPluginRegisterer
{
public:
    /** Construct a registerer and register the Impl class. @version 1.10.0 */
    IOPluginRegisterer();
};

}

#include "ioPluginFactory.ipp" // template implementation

#endif // LUNCHBOX_IOPLUGINFACTORY_H
