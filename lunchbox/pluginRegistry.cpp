
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
 *
 * This file is part of Collage <https://github.com/Eyescale/Collage>
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

#include "pluginRegistry.h"

#include "compressorInfo.h"
#include "debug.h"
#include "file.h"
#include "log.h"
#include "plugin.h"
#include "pluginVisitor.h"
#include "stdExt.h"

#ifdef _MSC_VER
#  include <lunchbox/os.h> // GetModuleFileName
#  include <direct.h>
#  define getcwd _getcwd
#else
#  include <unistd.h>   // for getcwd
#endif

#ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
#endif

namespace lunchbox
{
namespace detail
{
class PluginRegistry
{
public:
    PluginRegistry()
    {
        char* env = getenv( "EQ_PLUGIN_PATH" );
        std::string envString( env ? env : "" );

        if( envString.empty( ))
        {
            char cwd[MAXPATHLEN];
            directories.push_back( getcwd( cwd, MAXPATHLEN ));

#ifdef _WIN32
            if( GetModuleFileName( 0, cwd, MAXPATHLEN ) > 0 )
                directories.push_back( lunchbox::getDirname( cwd ));
#else
#  ifdef Darwin
            env = getenv( "DYLD_LIBRARY_PATH" );
#  else
            env = getenv( "LD_LIBRARY_PATH" );
#  endif
        if( env )
            envString = env;
#  endif
        }

#ifdef _WIN32
        const char separator = ';';
#else
        const char separator = ':';
#endif

        while( !envString.empty( ))
        {
            size_t nextPos = envString.find( separator );
            if ( nextPos == std::string::npos )
                nextPos = envString.size();

            std::string path = envString.substr( 0, nextPos );
            if ( nextPos == envString.size( ))
                envString = "";
            else
                envString = envString.substr( nextPos + 1, envString.size() );

            if( !path.empty( ))
                directories.push_back( path );
        }
    }

    ~PluginRegistry()
    {
        LBASSERTINFO( plugins.empty(), "Plugin registry not de-initialized" );
    }

    Strings directories;
    Plugins plugins;
};
}

PluginRegistry::PluginRegistry()
    : impl_( new detail::PluginRegistry )
{}

PluginRegistry::~PluginRegistry()
{
    delete impl_;
}

const Strings& PluginRegistry::getDirectories() const
{
    return impl_->directories;
}

void  PluginRegistry::addDirectory( const std::string& path )
{
    impl_->directories.push_back( path );
}

void PluginRegistry::removeDirectory( const std::string& path )
{
    Strings::iterator i = stde::find( impl_->directories, path );
    if( i != impl_->directories.end( ))
        impl_->directories.erase( i );
}

bool PluginRegistry::addLunchboxPlugins()
{
#ifdef LUNCHBOX_DSO_NAME
    return
        addPlugin( LUNCHBOX_DSO_NAME ) || // Found by LDD
        // Hard-coded compile locations as backup:
        addPlugin( std::string( LUNCHBOX_BUILD_DIR ) + "lib/" +
                   LUNCHBOX_DSO_NAME ) ||
#  ifdef NDEBUG
        addPlugin( std::string( LUNCHBOX_BUILD_DIR ) +
                   "lib/Release/" + LUNCHBOX_DSO_NAME )
#  else
        addPlugin( std::string( LUNCHBOX_BUILD_DIR ) + "lib/Debug/" +
                   LUNCHBOX_DSO_NAME )
#  endif
        ;
#endif
    return false;
}

void PluginRegistry::init()
{
    // for each directory
    for( StringsCIter i = impl_->directories.begin();
         i != impl_->directories.end(); ++i )
    {
        const std::string& dir = *i;
        LBLOG( LOG_PLUGIN ) << "Searching plugins in " << dir << std::endl;

#ifdef _WIN32
        const Strings& files = searchDirectory( dir, ".*Compressor.*\\.dll" );
        const char DIRSEP = '\\';
#elif defined (Darwin)
        const Strings& files = searchDirectory( dir,
                                                "lib.*Compressor.*\\.dylib" );
        const char DIRSEP = '/';
#else
        const Strings& files = searchDirectory( dir, "lib.*Compressor.*\\.so" );
        const char DIRSEP = '/';
#endif
        // for each file found in the directory
        for( StringsCIter j = files.begin(); j != files.end(); ++j )
        {
            // build path + name of library
            const std::string libraryName = dir.empty() ? *j : dir+DIRSEP+*j;
            addPlugin( libraryName );
        }
    }
}

namespace
{
Plugin* _loadPlugin( const std::string& filename, const Strings& directories )
{
    if( filename.size() < 3 )
        return 0;

    Plugin* plugin = new Plugin( filename );
    if( plugin->isGood( ))
        return plugin;
    delete plugin;

    if( filename[0] == '/' || filename[1] == ':' /* Win drive letter */ )
        return 0;

    for( StringsCIter i = directories.begin(); i != directories.end(); ++i )
    {
        const std::string& dir = *i;
        plugin = new Plugin( dir + "/" + filename );
        if( plugin->isGood( ))
            return plugin;
        delete plugin;
    }
    return 0;
}
}

bool PluginRegistry::addPlugin( const std::string& filename )
{
    Plugin* plugin = _loadPlugin( filename, impl_->directories );
    if( !plugin )
        return false;

    const CompressorInfos& infos = plugin->getInfos();
    for( PluginsCIter i = impl_->plugins.begin(); i != impl_->plugins.end(); ++i)
    {
        const CompressorInfos& infos2 = (*i)->getInfos();

        // Simple test to avoid loading the same dll twice
        if( infos.front().name == infos2.front().name )
        {
            delete plugin;
            return true;
        }
    }

    impl_->plugins.push_back( plugin );
    LBLOG( LOG_PLUGIN ) << "Found " << plugin->getInfos().size()
                        << " compression engines in " << filename << std::endl;
    return true;
}

void PluginRegistry::exit()
{
    for( PluginsCIter i = impl_->plugins.begin(); i != impl_->plugins.end(); ++i)
    {
        Plugin* plugin = *i;
        delete plugin;
    }

    impl_->plugins.clear();
}

namespace
{
template< class P,  class I > class Finder : public PluginVisitorT< P, I >
{
public:
    Finder( const uint32_t name ) : plugin( 0 ), name_( name ) {}
    virtual VisitorResult visit( P& candidate, I& info )
    {
        if( info.name != name_ )
            return TRAVERSE_CONTINUE;

        plugin = &candidate;
        return TRAVERSE_TERMINATE;
    }

    P* plugin;
private:
    const uint32_t name_;
};
}

Plugin* PluginRegistry::findPlugin( const uint32_t name )
{
    Finder< Plugin, EqCompressorInfo > finder( name );
    accept( finder );
    return finder.plugin;
}

const Plugin* PluginRegistry::findPlugin( const uint32_t name ) const
{
    Finder< const Plugin, const EqCompressorInfo > finder( name );
    accept( finder );
    return finder.plugin;
}

VisitorResult PluginRegistry::accept( PluginVisitor& visitor )
{
    VisitorResult result = TRAVERSE_CONTINUE;
    for( PluginsCIter i = impl_->plugins.begin(); i != impl_->plugins.end(); ++i )
        switch( (*i)->accept( visitor ))
        {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;
        case TRAVERSE_PRUNE:
            result = TRAVERSE_PRUNE;
        default:
            break;
        }

    return result;
}
VisitorResult PluginRegistry::accept( ConstPluginVisitor& visitor ) const
{
    VisitorResult result = TRAVERSE_CONTINUE;
    for( PluginsCIter i = impl_->plugins.begin(); i != impl_->plugins.end(); ++i )
        switch( (*i)->accept( visitor ))
        {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;
        case TRAVERSE_PRUNE:
            result = TRAVERSE_PRUNE;
        case TRAVERSE_CONTINUE:
            break;
        }

    return result;
}

const Plugins& PluginRegistry::getPlugins() const
{
    return impl_->plugins;
}

}
