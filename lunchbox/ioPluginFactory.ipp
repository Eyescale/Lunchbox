
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

namespace lunchbox
{

template< typename IOPluginT, typename InitDataT >
IOPluginAbstractFactory< IOPluginT, InitDataT >&
    IOPluginAbstractFactory< IOPluginT, InitDataT >::getInstance()
{
    static IOPluginAbstractFactory< IOPluginT, InitDataT > factory;
    return factory;
}

template< typename IOPluginT, typename InitDataT >
IOPluginT* IOPluginAbstractFactory< IOPluginT, InitDataT >::create(
                                                 const InitDataT& initData )
{
    for( typename IOPlugins::const_iterator i = _plugins.begin();
         i != _plugins.end(); ++i )
    {
        if( i->handles( initData ))
            return i->constructor( initData );
    }

    LBTHROW( std::runtime_error( "No plugin implementation available for " +
                                 boost::lexical_cast<std::string>( initData )));
}

template< typename IOPluginT, typename InitDataT >
void IOPluginAbstractFactory< IOPluginT, InitDataT >::registerPlugin(
                                           const IOPlugin< IOPluginT,
                                           InitDataT >& plugin )
{
    _plugins.push_back( plugin );
}

template< typename IOPluginT, typename InitDataT >
void IOPluginAbstractFactory< IOPluginT, InitDataT >::unregisterAllPlugins()
{
    _plugins.clear();
}

template< typename IOPluginT, typename InitDataT >
IOPlugin< IOPluginT, InitDataT >::IOPlugin( const Constructor& constructor_,
                                            const HandlesFunc& handles_ )
    : constructor( constructor_ )
    , handles( handles_ )
{}

template< typename Impl >
IOPluginRegisterer< Impl, true >::IOPluginRegisterer( )
{
    IOPlugin< typename Impl::IOPluginT, typename Impl::InitDataT > plugin (
                boost::bind( boost::factory< Impl* >(), _1 ),
                boost::bind( &Impl::handles, _1 ));
    typedef IOPluginAbstractFactory< typename Impl::IOPluginT,
                                     typename Impl::InitDataT >
        _PluginFactory;
    _PluginFactory::getInstance().registerPlugin( plugin );
}

template< typename Impl >
IOPluginRegisterer< Impl, false >::IOPluginRegisterer( )
{
    IOPlugin< typename Impl::IOPluginT > plugin (
                boost::bind( boost::factory< Impl* >(), _1 ),
                boost::bind( &Impl::handles, _1 ));

    typedef IOPluginAbstractFactory< typename Impl::IOPluginT >
        _PluginFactory;

    _PluginFactory::getInstance().registerPlugin( plugin );
}



}
