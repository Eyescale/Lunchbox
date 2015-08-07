
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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
    typedef PluginInterface PluginT;
    virtual ~PluginInterface() {}
    virtual int getValue() = 0;
};

class TypedPluginInterface
{
public:
    typedef TypedPluginInterface PluginT;
    typedef InitData InitDataT;
    virtual ~TypedPluginInterface() {}
    virtual int getValue() = 0;
};

class MyPlugin : public PluginInterface
{
public:
    explicit MyPlugin( const servus::URI& ) {}
    static bool handles( const servus::URI& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class MyTypedPlugin : public TypedPluginInterface
{
public:
    explicit MyTypedPlugin( const InitData&  ) {}
    static bool handles( const InitData& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class MyDummyPlugin : public PluginInterface
{
public:
    explicit MyDummyPlugin( const servus::URI& ) {}
    static bool handles( const servus::URI& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

class MyTypedDummyPlugin : public TypedPluginInterface
{
public:
    explicit MyTypedDummyPlugin( const InitData&  ) {}
    static bool handles( const InitData& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

typedef lunchbox::PluginFactory< PluginInterface> MyPluginFactory;
typedef lunchbox::PluginFactory< TypedPluginInterface,
                                 InitData > MyTypedPluginFactory;

typedef boost::scoped_ptr< PluginInterface > PluginInterfacePtr;
typedef boost::scoped_ptr< TypedPluginInterface > TypedPluginInterfacePtr;

void tryCreatePlugin( PluginInterfacePtr& plugin )
{
    MyPluginFactory& factory = MyPluginFactory::getInstance();
    plugin.reset( factory.create( servus::URI( "XYZ" )));
}

void tryCreateTypedPlugin( TypedPluginInterfacePtr& plugin )
{
    MyTypedPluginFactory& factory = MyTypedPluginFactory::getInstance();
    plugin.reset( factory.create( InitData() ));
}

BOOST_AUTO_TEST_CASE( testWhenNoPluginIsRegisteredCreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().deregisterAll();

    PluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenNoTypedPluginIsRegisteredCreateThrowsRuntimeErr )
{
    MyTypedPluginFactory::getInstance().deregisterAll();

    TypedPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreateTypedPlugin( plugin ), std::runtime_error );
}


BOOST_AUTO_TEST_CASE( testWhenPluginRegistererIsInstantiatedPluginIsRegistered )
{
    MyPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyPlugin > registerer;

    PluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreatePlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE(
                testWhenTypedPluginRegistererIsInstantiatedPluginIsRegistered )
{
    MyTypedPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyTypedPlugin > registerer;

    TypedPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreateTypedPlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE( testWhenPluginsDontHandleURICreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyDummyPlugin > registerer;

    PluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenTypedPlginsDontHandleURICreateThrowsRuntimeError )
{
    MyTypedPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyTypedDummyPlugin > registerer;

    TypedPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreateTypedPlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenOnePluginHandlesURICreateInstanciesCorrectType )
{
    MyPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyDummyPlugin > registerer1;
    lunchbox::PluginRegisterer< MyPlugin > registerer2;

    PluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreatePlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE( testWhenOneTypedPluginHandlesURICreateInstCorrectType )
{
    MyTypedPluginFactory::getInstance().deregisterAll();

    lunchbox::PluginRegisterer< MyTypedDummyPlugin > registerer1;
    lunchbox::PluginRegisterer< MyTypedPlugin > registerer2;

    TypedPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreateTypedPlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}
