
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

namespace lunchbox
{
template< typename PluginT, typename InitDataT >
PluginFactory< PluginT, InitDataT >&
PluginFactory< PluginT, InitDataT >::getInstance()
{
    static PluginFactory< PluginT, InitDataT > factory;
    return factory;
}

template< typename PluginT, typename InitDataT >
PluginT* PluginFactory< PluginT, InitDataT >::create( const InitDataT& initData )
{
    BOOST_FOREACH( PluginHolder& plugin, _plugins )
        if( plugin.handles( initData ))
            return plugin.constructor( initData );

    LBTHROW( std::runtime_error( "No plugin implementation available for " +
                                 boost::lexical_cast<std::string>( initData )));
}

template< typename PluginT, typename InitDataT >
void PluginFactory< PluginT, InitDataT >::register_(
    const Plugin< PluginT, InitDataT >& plugin )
{
    _plugins.push_back( plugin );
}

template< typename PluginT, typename InitDataT >
bool PluginFactory< PluginT, InitDataT >::deregister(
    const Plugin< PluginT, InitDataT >& plugin )
{
    typename Plugins::iterator i =
        std::find( _plugins.begin(), _plugins.end(), plugin );
    if( i == _plugins.end( ))
        return false;

    _plugins.erase( i );
    return true;
}

template< typename PluginT, typename InitDataT >
void PluginFactory< PluginT, InitDataT >::deregisterAll()
{
    _plugins.clear();
    BOOST_FOREACH( typename PluginMap::value_type& plugin, _libraries )
        delete plugin.first;
    _libraries.clear();
}

template< typename PluginT, typename InitDataT >
DSOs PluginFactory< PluginT, InitDataT >::load( const int version,
                                                const Strings& paths,
                                                const std::string& pattern )
{
    Strings unique = paths;
    lunchbox::usort( unique );

    DSOs result;
    BOOST_FOREACH( const std::string& path, unique )
        _load( result, version, path, pattern );
    return result;
}

template< typename PluginT, typename InitDataT >
DSOs PluginFactory< PluginT, InitDataT >::load( const int version,
                                                const std::string& path,
                                                const std::string& pattern )
{
    DSOs loaded;
    _load( loaded, version, path, pattern );
    return loaded;
}

template< typename PluginT, typename InitDataT >
void PluginFactory< PluginT, InitDataT >::_load( DSOs& result,
                                                 const int version,
                                                 const std::string& path,
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

    BOOST_FOREACH( const std::string& lib, libs )
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
            LBINFO << "Disable " << lib << ": "
                   << ( getVersion ? "" :
                        "Symbol for LunchboxPluginGetVersion missing " )
                   << ( registerFunc ? "" :
                        "Symbol for LunchboxPluginRegister missing " );
            if( getVersion && !matchesVersion )
                LBINFO << "Plugin version " << getVersion() << " does not"
                       << " match application version " << version;
            LBINFO << std::endl;

            delete dso;
            continue;
        }

        if( registerFunc( ))
        {
            _libraries.insert( std::make_pair( dso, _plugins.back( )));
            result.push_back( dso );
            LBINFO << "Enabled " << lib << std::endl;
        }
        else
            delete dso;
    }
}

template< typename PluginT, typename InitDataT >
bool PluginFactory< PluginT, InitDataT >::unload( DSO* dso )
{
    typename PluginMap::iterator i = _libraries.find( dso );
    if( i == _libraries.end( ))
        return false;

    delete i->first;
    const bool ret = deregister( i->second );
    _libraries.erase( i );
    return ret;
}
}
