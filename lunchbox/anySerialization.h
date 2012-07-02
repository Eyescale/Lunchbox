
/* Copyright (c) 2012, EFPL/Blue Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *
 * This file is part of DASH <https://github.com/BlueBrain/dash>
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

#ifndef DASH_DETAIL_ANYSERIALIZATION_H
#define DASH_DETAIL_ANYSERIALIZATION_H

#include "any.h"

#include <lunchbox/uint128_t.h>

#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

#include <boost/serialization/export.hpp>
#include <boost/serialization/extended_type_info.hpp>


/**
 * Declares the given class to be serializable within a dash::detail::Any.
 * User is supposed to use this macro on global scope and in the compilation
 * unit where this class is to be serialized.
 */
#define SERIALIZABLEANY( CLASS ) \
    BOOST_CLASS_EXPORT( dash::detail::Any::holder< CLASS > )

/**
 * Enables the given archive to serialize all POD types through a
 * dash::detail::Any. Without this registration, user may experience exceptions
 * being thrown complaining about unregistered classes.
 */
#define ANYARCHIVE( ar )   \
    dash::detail::registerAnyPodTypes( ar );


namespace dash
{
namespace detail
{

/** List of supported POD types for dash::detail::Any serialization. */
typedef boost::mpl::list< short, unsigned short,
                          int, unsigned int,
                          long, unsigned long,
                          bool, float, double,
                          std::string, lunchbox::uint128_t > podTypes;

/**
 * Registers the given type for serializing it inside a dash::detail::Any
 * through the given archive.
 */
template< class T, class Archive >
void registerAnyType( Archive& ar )
{
    ar.template register_type< dash::detail::Any::holder< T > >();
}

/**
 * Utility struct for registering types for dash::detail::Any from a type list.
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
 * dash::detail::Any through the given archive.
 */
template< class Archive, class TypeList >
void registerAnyTypes( Archive& ar )
{
    boost::mpl::for_each< TypeList >( registerWrapper< Archive >( ar ) );
}

/**
 * Registers POD types for serializing them inside a dash::detail::Any through
 * the given archive.
 */
template< class Archive >
void registerAnyPodTypes( Archive& ar )
{
    registerAnyTypes< Archive, podTypes >( ar );
}

/**
 * Serializes the given object which can be a dash::detail::Any through the
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
}


#endif
