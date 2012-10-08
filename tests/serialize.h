
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef LUNCHBOX_SERIALIZE_H
#define LUNCHBOX_SERIALIZE_H

#include "test.h"

#include <lunchbox/anySerialization.h>

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#pragma warning( push )
#pragma warning( disable: 4996 )
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#pragma warning( pop )


template< class T >
void textSave( const T& object, std::ostream& os )
{
    lunchbox::saveAny< boost::archive::text_oarchive >( object, os );
}

template< class T >
void textLoad( T& object, std::istream& is )
{
    lunchbox::loadAny< boost::archive::text_iarchive >( object, is );
}

template< class T >
void binarySave( const T& object, std::ostream& os )
{
    lunchbox::saveAny< boost::archive::binary_oarchive >( object, os );
}

template< class T >
void binaryLoad( T& object, std::istream& is )
{
    lunchbox::loadAny< boost::archive::binary_iarchive >( object, is );
}

template< class T >
void textSerialize( const T& object, T& loadedObject )
{
    std::stringstream stream;
    textSave( object, stream );
    textLoad( loadedObject, stream );
}

template< class T >
void textSerializeAndTest( const T& object )
{
    T loadedObject;
    textSerialize( object, loadedObject );
    TEST( object == loadedObject );
}

template< class T >
void binarySerialize( const T& object, T& loadedObject )
{
    std::stringstream stream;
    binarySave( object, stream );
    binaryLoad( loadedObject, stream );
}

template< class T >
void binarySerializeAndTest( const T& object )
{
    T loadedObject;
    binarySerialize( object, loadedObject );
    TEST( object == loadedObject );
}


struct Foo
{
    bool operator == ( const Foo& rhs ) const
    {
        if( this == &rhs )
            return true;

        return i == rhs.i && f == rhs.f && b == rhs.b && s == rhs.s;
    }

    bool operator != ( const Foo& rhs ) const
    {
        return !( *this == rhs );
    }

    int i;
    float f;
    bool b;
    std::string s;

    template< class Archive >
    void serialize( Archive & ar, const unsigned int version )
    {
        ar & i;
        ar & f;
        ar & b;
        ar & s;
    }
};

SERIALIZABLEANY( Foo )


#endif
