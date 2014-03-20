
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef LUNCHBOX_PLUGINREGISTRY_H
#define LUNCHBOX_PLUGINREGISTRY_H

#include <lunchbox/api.h>
#include <lunchbox/types.h>
#include <lunchbox/visitorResult.h> // enum
#include <boost/noncopyable.hpp>
#include <string>

namespace lunchbox
{
namespace detail { class PluginRegistry; }

/**
 * A registry for loaded plugins.
 *
 * Downstream projects such as Collage and Equalizer use and initialize a global
 * plugin registry in their respective initialization calls by adding
 * directories in co::init(). This internal plugin registry can be obtained
 * using co::Global::getPluginRegistry().
 *
 * Example: @include tests/compressor.cpp
 */
class PluginRegistry : public boost::noncopyable
{
public:
    /** Construct a new plugin registry. @version 1.7.1 */
    LUNCHBOX_API PluginRegistry();

    /** Destruct this plugin registry. @version 1.7.1 */
    LUNCHBOX_API ~PluginRegistry();

    /**
     * Add a new directory to search for compressor DSOs during init().
     * @version 1.0 in Collage
     * @version 1.7.1 in Lunchbox
     */
    LUNCHBOX_API void addDirectory( const std::string& path );

    /** Remove a plugin directory. @version 1.0 */
    LUNCHBOX_API void removeDirectory( const std::string& path );

    /**
     * @return all directories to search for compressor DSOs during init().
     * @version 1.0 in Collage
     * @version 1.7.1 in Lunchbox
     */
    LUNCHBOX_API const Strings& getDirectories() const;

    /**
     * Add the lunchbox library plugins to this registry.
     * @return true on success, false otherwise.
     * @version 1.7.1
     */
    LUNCHBOX_API bool addLunchboxPlugins();

    /** @internal Search all plugin directories and register found DSOs */
    LUNCHBOX_API void init();

    /** @internal Exit all DSOs and free all plugins */
    LUNCHBOX_API void exit();

    /**
     * Visit all plugins and compressors.
     *
     * @return TRAVERSE_TERMINATE immediately when one visit method returned
     *         terminate, TRAVERSE_PRUNE if at least one visit method returned
     *         prune, TRAVERSE_CONTINUE otherwise.
     * @version 1.7.1
     */
    LUNCHBOX_API VisitorResult accept( PluginVisitor& visitor );

    /** Visit all plugins and compressors. @version 1.7.1 */
    LUNCHBOX_API VisitorResult accept( ConstPluginVisitor& visitor ) const;

    /** @internal @return all registered compressor plugins */
    LUNCHBOX_API const Plugins& getPlugins() const;

    /** @internal @return the plugin containing the given compressor. */
    LUNCHBOX_API Plugin* findPlugin( const uint32_t name );

    /** @internal @return the plugin containing the given compressor. */
    LUNCHBOX_API const Plugin* findPlugin( const uint32_t name ) const;

    /** @internal Add a single DSO before init(). @return true if found. */
    LUNCHBOX_API bool addPlugin( const std::string& filename );

private:
    detail::PluginRegistry* const impl_;
};
}
#endif // LUNCHBOX_PLUGINREGISTRY_H
