
/* Copyright (c) 2012, Daniel Nachbaur <daniel.nachbaur@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#ifndef LUNCHBOX_ANYSERIALIZATION_H
#define LUNCHBOX_ANYSERIALIZATION_H

#include <lunchbox/any.h>
#include <lunchbox/stdExt.h>

#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

#include <boost/serialization/export.hpp>
#include <boost/serialization/extended_type_info.hpp>


/**
 * Declares the given class to be serializable within a lunchbox::Any.
 * User is supposed to use this macro on global scope and in the compilation
 * unit where this class is to be serialized.
 */
#define SERIALIZABLEANY( CLASS ) \
    BOOST_CLASS_EXPORT( lunchbox::Any::holder< CLASS > )

/**
 * Enables the given archive to serialize all POD types through a
 * lunchbox::Any. Without this registration, user may experience exceptions
 * being thrown complaining about unregistered classes.
 */
#define ANYARCHIVE( ar )   \
    lunchbox::registerAnyPodTypes( ar );


namespace lunchbox
{

/** List of supported POD types for lunchbox::Any serialization. */
typedef boost::mpl::list< int8_t, uint8_t,
                          int16_t, uint16_t,
                          int32_t, uint32_t,
                          int64_t, uint64_t,
                          float, double,
                          bool, std::string, uint128_t > podTypes;

/**
 * Registers the given type for serializing it inside a lunchbox::Any
 * through the given archive.
 */
template< class T, class Archive >
void registerAnyType( Archive& ar )
{
    ar.template register_type< Any::holder< T > >();
}

/**
 * Utility struct for registering types for lunchbox::Any from a type list.
 * @internal
 */
template< class Archive >
struct registerWrapper
{
    registerWrapper( Archive& ar ) : ar_( ar ) {}
    Archive& ar_;

    template< typename U >
    void operator()( U )
    {
        registerAnyType< U >( ar_ );
    }
};

/**
 * Registers the types from the given type list for serializing it inside a
 * lunchbox::Any through the given archive.
 */
template< class Archive, class TypeList >
void registerAnyTypes( Archive& ar )
{
    boost::mpl::for_each< TypeList >( registerWrapper< Archive >( ar ) );
}

/**
 * Registers POD types for serializing them inside a lunchbox::Any through
 * the given archive.
 */
template< class Archive >
void registerAnyPodTypes( Archive& ar )
{
    registerAnyTypes< Archive, podTypes >( ar );
}

/**
 * Serializes the given object which can be a lunchbox::Any through the
 * given archive type to/from the given stream.
 */
template< class Archive, class Object, class Stream >
void serializeAny( Object& object, Stream& stream )
{
    Archive ar( stream );
    ANYARCHIVE( ar );
    ar & object;
}

}

#endif
