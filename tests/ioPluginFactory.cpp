
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

#define BOOST_TEST_MODULE IOPluginFactory

#include <lunchbox/types.h>
#include <lunchbox/ioPluginFactory.h>
#include <lunchbox/uri.h>

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#define VALID_VALUE    10
#define INVALID_VALUE   0

struct InitData
{
    lunchbox::URI uri;
};

namespace boost
{
template<> inline std::string lexical_cast( const InitData& data )
{
    return lexical_cast< std::string >( data.uri );
}
}

class IOPluginInterface
{
public:
    typedef IOPluginInterface IOPluginT;
    virtual ~IOPluginInterface() {}
    virtual int getValue() = 0;
};

class IOTypedPluginInterface
{
public:
    typedef IOTypedPluginInterface IOPluginT;
    typedef InitData InitDataT;
    virtual ~IOTypedPluginInterface() {}
    virtual int getValue() = 0;
};

class MyPlugin : public IOPluginInterface
{
public:
    MyPlugin( const lunchbox::URI& ) {}
    static bool handles( const lunchbox::URI& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class MyTypedPlugin : public IOTypedPluginInterface
{
public:
    MyTypedPlugin( const InitData&  ) {}
    static bool handles( const InitData& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class MyDummyPlugin : public IOPluginInterface
{
public:
    MyDummyPlugin( const lunchbox::URI& ) {}
    static bool handles( const lunchbox::URI& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

class MyTypedDummyPlugin : public IOTypedPluginInterface
{
public:
    MyTypedDummyPlugin( const InitData&  ) {}
    static bool handles( const InitData& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

typedef lunchbox::IOPluginAbstractFactory< IOPluginInterface> MyPluginFactory;
typedef lunchbox::IOPluginAbstractFactory< IOTypedPluginInterface, InitData >
                                                           MyTypedPluginFactory;

typedef boost::scoped_ptr< IOPluginInterface > IOPluginInterfacePtr;
typedef boost::scoped_ptr< IOTypedPluginInterface > IOTypedPluginInterfacePtr;

void tryCreatePlugin( IOPluginInterfacePtr& plugin )
{
    MyPluginFactory& factory = MyPluginFactory::getInstance();
    plugin.reset( factory.create( lunchbox::URI( "XYZ" )));
}

void tryCreateTypedPlugin( IOTypedPluginInterfacePtr& plugin )
{
    MyTypedPluginFactory& factory = MyTypedPluginFactory::getInstance();
    plugin.reset( factory.create( InitData() ));
}

BOOST_AUTO_TEST_CASE( testWhenNoPluginIsRegisteredCreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    IOPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenNoTypedPluginIsRegisteredCreateThrowsRuntimeErr )
{
    MyTypedPluginFactory::getInstance().unregisterAllPlugins();

    IOTypedPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreateTypedPlugin( plugin ), std::runtime_error );
}


BOOST_AUTO_TEST_CASE( testWhenPluginRegistererIsInstantiatedPluginIsRegistered )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyPlugin > registerer;

    IOPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreatePlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE(
                testWhenTypedPluginRegistererIsInstantiatedPluginIsRegistered )
{
    MyTypedPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyTypedPlugin > registerer;

    IOTypedPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreateTypedPlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE( testWhenPluginsDontHandleURICreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyDummyPlugin > registerer;

    IOPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenTypedPlginsDontHandleURICreateThrowsRuntimeError )
{
    MyTypedPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyTypedDummyPlugin > registerer;

    IOTypedPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreateTypedPlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenOnePluginHandlesURICreateInstanciesCorrectType )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyDummyPlugin > registerer1;
    lunchbox::IOPluginRegisterer< MyPlugin > registerer2;

    IOPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreatePlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE( testWhenOneTypedPluginHandlesURICreateInstCorrectType )
{
    MyTypedPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyTypedDummyPlugin > registerer1;
    lunchbox::IOPluginRegisterer< MyTypedPlugin > registerer2;

    IOTypedPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreateTypedPlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}
