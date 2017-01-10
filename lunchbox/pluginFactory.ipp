
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

namespace lunchbox
{
template< typename T > PluginFactory< T >& PluginFactory< T >::getInstance()
{
    static PluginFactory< T > factory;
    return factory;
}

template< typename T > PluginFactory< T >::~PluginFactory()
{
    // Do not do this: dtor is called in atexit(), at which point the other DSOs
    // might be unloaded already, causing dlclose to trip. It's pointless
    // anyways, we're in atexit, so the OS will dispose the DSOs for us anyways.
    // Let's call this a static deinitializer fiasco.
    //   deregisterAll(); // unload the DSO libraries
}

template< typename T >
bool PluginFactory< T >::handles( const typename T::InitDataT& initData )
{
    for( auto& plugin : _plugins )
        if( plugin.handles( initData ))
            return true;
    return false;
}

template< typename T >
T* PluginFactory< T >::create( const typename T::InitDataT& initData )
{
    for( auto& plugin : _plugins )
        if( plugin.handles( initData ))
            return plugin.construct( initData );

    LBTHROW( std::runtime_error( "No plugin implementation available for " +
                                 std::to_string( initData )));
}

template< typename T >
void PluginFactory< T >::register_( const PluginT& plugin )
{
    _plugins.push_back( plugin );
}

template< typename T >
bool PluginFactory< T >::deregister( const PluginT& plugin )
{
    typename Plugins::iterator i = std::find( _plugins.begin(), _plugins.end(),
                                              plugin );
    if( i == _plugins.end( ))
        return false;

    _plugins.erase( i );
    return true;
}

template< typename T > void PluginFactory< T >::deregisterAll()
{
    _plugins.clear();
    for( auto& plugin : _libraries )
        delete plugin.first;
    _libraries.clear();
}

template< typename T > std::string PluginFactory< T >::getDescriptions() const
{
    std::string descriptions;
    for( const auto& plugin : _plugins )
        descriptions += (descriptions.empty() ? "" : "\n\n" ) +
                        plugin.getDescription();
    return descriptions;
}

template< typename T >
void PluginFactory< T >::load( const int version, const Strings& paths,
                               const std::string& pattern )
{
    Strings unique = paths;
    lunchbox::usort( unique );

    for( const auto& path : unique )
        load( version, path, pattern );
}

template< typename T >
void PluginFactory< T >::load( const int version, const std::string& path,
                               const std::string& pattern )
{
#ifdef _MSC_VER
    const std::string regex( pattern + ".dll" );
#elif __APPLE__
    const std::string regex( "lib" + pattern + ".dylib" );
#else
    const std::string regex( "lib" + pattern + ".so" );
#endif
    const Strings& libs = searchDirectory( path, regex );

    for( const auto& lib : libs )
    {
        lunchbox::DSO* dso = new lunchbox::DSO( path + "/" + lib );
        if( !dso->isOpen())
        {
            delete dso;
            continue;
        }

        typedef int( *GetVersion_t )();
        typedef bool( *Register_t )();

        GetVersion_t getVersion = dso->getFunctionPointer< GetVersion_t >(
            "LunchboxPluginGetVersion" );
        Register_t registerFunc = dso->getFunctionPointer< Register_t >(
            "LunchboxPluginRegister" );
        const bool matchesVersion = getVersion && (getVersion() == version);

        if( !getVersion || !registerFunc || !matchesVersion )
        {
            LBERROR << "Disable " << lib << ": "
                    << ( getVersion ? "" :
                        "Symbol for LunchboxPluginGetVersion missing " )
                    << ( registerFunc ? "" :
                        "Symbol for LunchboxPluginRegister missing " );
            if( getVersion && !matchesVersion )
                LBERROR << "Plugin version " << getVersion() << " does not"
                        << " match application version " << version;
            LBERROR << std::endl;

            delete dso;
            continue;
        }

        if( registerFunc( ))
        {
            _libraries.insert( std::make_pair( dso, _plugins.back( )));
            LBINFO << "Loaded plugin " << lib << std::endl;
        }
        else
            delete dso;
    }
}
}
