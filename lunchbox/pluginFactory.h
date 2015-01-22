
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

#ifndef LUNCHBOX_PLUGINFACTORY_H
#define LUNCHBOX_PLUGINFACTORY_H

#include <lunchbox/types.h>
#include <lunchbox/debug.h> // LBTHROW
#include <lunchbox/dso.h> // used inline
#include <lunchbox/file.h> // searchDirectory() used inline
#include <lunchbox/uri.h> // Default template type

#include <boost/noncopyable.hpp> // base class
#include <boost/lexical_cast.hpp> // used inline
#include <boost/unordered_map.hpp>

namespace lunchbox
{

/**
 * Factory for Plugin classes.
 *
 * The PluginFactory selects the a plugin for a given InitDataT, based on a
 * plugin's handles() function. In case a InitDataT can be handled by multiple
 * plugins, which plugin is chosen is undefined.
 *
 * This class has been designed as a singleton to allow for link time plugin
 * registration, but nothing prevents an application from registering new types
 * at run time.
 *
 * To do the registration of a plugin during the static initialization phase
 * use the PluginRegisterer.
 *
 * Example: @include tests/pluginFactory.cpp
 *
 * @version 1.11.0
 */
template< class PluginT, class InitDataT = URI >
class PluginFactory : public boost::noncopyable
{
public:
    typedef Plugin< PluginT, InitDataT > PluginHolder;
    typedef std::vector< PluginHolder > Plugins;

    /** Get the single class instance. @version 1.11.0 */
    static PluginFactory& getInstance();

    /**
     * Create a plugin instance.
     * @param initData The initData passed to the plugin constructor.
     * @return A new PluginT instance. The user is responsible for deleting
     *         the returned object.
     * @throws std::runtime_error if no plugin can handle the initData.
     * @version 1.11.0
     */
    PluginT* create( const InitDataT& initData );

    /** Register a plugin type. @version 1.11.0 */
    void register_( const Plugin< PluginT, InitDataT >& plugin );

    /** Deregister a plugin type. @version 1.11.0 */
    bool deregister( const Plugin< PluginT, InitDataT >& plugin );

    /** Unregister all plugin types. @version 1.11.0 */
    void deregisterAll();

    /** @name Automatic loading of plugin DSOs. */
    //@{
    /**
     * Load all compatible plugin libraries from a directory matching a pattern.
     *
     * The pattern is the core name of the library, and is extended by the
     * system-specific shared library suffix and postfix. The plugin has to
     * implement the C functions 'int LunchboxPluginGetVersion()' and 'bool
     * LunchboxPluginRegister()'. Only plugins with the same ABI version as the
     * given one are registered.
     *
     * @param version the current ABI version of the application loading the
     *                plugins.
     * @param path the directory to search for plugins.
     * @param pattern the core pattern of plugin names.
     * @return the loaded plugins, ownership remains with the PluginFactory.
     * @version 1.11.0
     * @sa getLibraryPath()
     */
    DSOs load( const int version, const std::string& path,
               const std::string& pattern );

    /**
     * Unload and deregister a previously loaded plugin
     *
     * @return true if the plugin was loaded, false on error.
     * @version 1.11.0
     */
    bool unload( DSO* dso );
    //@}

private:
    Plugins _plugins;

    typedef boost::unordered_map< DSO*, PluginHolder > PluginMap;
    PluginMap _libraries;
};

}

#include "pluginFactory.ipp" // template implementation

#endif // LUNCHBOX_PLUGINFACTORY_H
