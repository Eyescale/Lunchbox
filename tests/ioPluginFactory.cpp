
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

class IOPluginInterface
{
public:
    typedef IOPluginInterface IOPluginT;
    virtual ~IOPluginInterface() {}
    virtual int getValue() = 0;
};

class MyPlugin : public IOPluginInterface
{
public:
    MyPlugin( const lunchbox::URI& ) {}
    static bool handles( const lunchbox::URI& ) { return true; }
    int getValue() final { return VALID_VALUE; }
};

class MyDummyPlugin : public IOPluginInterface
{
public:
    MyDummyPlugin( const lunchbox::URI& ) {}
    static bool handles( const lunchbox::URI& ) { return false; }
    int getValue() final { return INVALID_VALUE; }
};

typedef lunchbox::IOPluginAbstractFactory< IOPluginInterface > MyPluginFactory;
typedef boost::scoped_ptr< IOPluginInterface > IOPluginInterfacePtr;

void tryCreatePlugin( IOPluginInterfacePtr& plugin )
{
    MyPluginFactory& factory = MyPluginFactory::getInstance();
    plugin.reset( factory.create( lunchbox::URI( "XYZ" )));
}

BOOST_AUTO_TEST_CASE( testWhenNoPluginIsRegisteredCreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    IOPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE( testWhenPluginRegistererIsInstantiatedPluginIsRegistered )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyPlugin > registerer;

    IOPluginInterfacePtr plugin;
    BOOST_REQUIRE_NO_THROW( tryCreatePlugin( plugin ));
    BOOST_CHECK_EQUAL( plugin->getValue(), VALID_VALUE );
}

BOOST_AUTO_TEST_CASE( testWhenPluginsDontHandleURICreateThrowsRuntimeError )
{
    MyPluginFactory::getInstance().unregisterAllPlugins();

    lunchbox::IOPluginRegisterer< MyDummyPlugin > registerer;

    IOPluginInterfacePtr plugin;
    BOOST_CHECK_THROW( tryCreatePlugin( plugin ), std::runtime_error );
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
