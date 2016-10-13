
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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

#define BOOST_TEST_MODULE PluginFactory

#include <lunchbox/types.h>
#include <lunchbox/pluginFactory.h>
#include <lunchbox/pluginRegisterer.h>
#include <servus/uri.h>

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#define VALID_VALUE    10
#define INVALID_VALUE   0

struct InitData
{
    InitData() : uri( "test://uri" ) {}
    servus::URI uri;
};

namespace boost
{
template<> inline std::string lexical_cast( const InitData& data )
{
    return lexical_cast< std::string >( data.uri );
}
}

class PluginInterface
{
public:
    typedef InitData InitDataT;
    typedef PluginInterface InterfaceT;

    virtual ~PluginInterface() {}
    virtual int getValue() = 0;
};

class Plugin : public PluginInterface
{
public:
    explicit Plugin( const InitData& ) {}
    static bool handles( const InitData& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class FalsePlugin : public PluginInterface
{
public:
    explicit FalsePlugin( const InitData& ) {}
    static bool handles( const InitData& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

typedef lunchbox::PluginFactory< PluginInterface > PluginFactory;
typedef boost::shared_ptr< PluginInterface > PluginInterfacePtr;

PluginInterfacePtr createPlugin()
{
    return PluginInterfacePtr(
        PluginFactory::getInstance().create( InitData( )));
}

BOOST_AUTO_TEST_CASE( throwNoneRegistered )
{
    PluginFactory::getInstance().deregisterAll();
    BOOST_CHECK_THROW( createPlugin(), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( creation )
{
    PluginFactory::getInstance().deregisterAll();
    lunchbox::PluginRegisterer< Plugin > registerer;
    PluginInterfacePtr plugin = createPlugin();

    BOOST_CHECK( plugin );
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}


BOOST_AUTO_TEST_CASE( throwHandlesFailure )
{
    PluginFactory::getInstance().deregisterAll();
    lunchbox::PluginRegisterer< FalsePlugin > registerer;

    BOOST_CHECK_THROW( createPlugin(), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( createCorrectVariant )
{
    PluginFactory::getInstance().deregisterAll();
    lunchbox::PluginRegisterer< FalsePlugin > registerer1;
    lunchbox::PluginRegisterer< Plugin > registerer2;
    PluginInterfacePtr plugin = createPlugin();

    BOOST_CHECK( plugin );
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}
