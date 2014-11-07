
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

template< typename IOPluginT >
IOPluginAbstractFactory< IOPluginT >&
    IOPluginAbstractFactory< IOPluginT >::getInstance()
{
    static IOPluginAbstractFactory< IOPluginT > factory;
    return factory;
}

template< typename IOPluginT >
IOPluginT* IOPluginAbstractFactory< IOPluginT >::create( const URI& uri )
{
    for( typename IOPlugins::const_iterator i = _plugins.begin();
         i != _plugins.end(); ++i )
    {
        if( i->handles( uri ))
            return i->constructor( uri );
    }
    const std::string uriString = boost::lexical_cast< std::string >( uri );
    LBTHROW( std::runtime_error(
        "No plugin implementation available for URI " + uriString ));
}

template< typename IOPluginT >
void IOPluginAbstractFactory< IOPluginT >::registerPlugin(
                                           const IOPlugin< IOPluginT >& plugin )
{
    _plugins.push_back( plugin );
}

template< typename IOPluginT >
void IOPluginAbstractFactory< IOPluginT >::unregisterAllPlugins()
{
    _plugins.clear();
}

template< typename IOPluginT >
IOPlugin< IOPluginT >::IOPlugin( const Constructor& constructor_,
                                 const HandlesFunc& handles_ )
    : constructor( constructor_ )
    , handles( handles_ )
{}

template< typename Impl >
IOPluginRegisterer< Impl >::IOPluginRegisterer()
{
    IOPlugin< typename Impl::IOPluginT > plugin (
                boost::bind( boost::factory< Impl* >(), _1 ),
                boost::bind( &Impl::handles, _1 ));
    typedef IOPluginAbstractFactory< typename Impl::IOPluginT> _PluginFactory;
    _PluginFactory::getInstance().registerPlugin( plugin );
}

}
