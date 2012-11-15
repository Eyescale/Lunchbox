
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

// Based on boost/any.hpp
// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef LUNCHBOX_ANY_H
#define LUNCHBOX_ANY_H

#include <lunchbox/api.h>
#include <lunchbox/debug.h>

#include <boost/type_traits/remove_reference.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>

// See boost/python/type_id.hpp
// TODO: add BOOST_TYPEID_COMPARE_BY_NAME to config.hpp
# if (defined(__GNUC__) && __GNUC__ >= 3) \
 || defined(_AIX) \
 || (   defined(__sgi) && defined(__host_mips)) \
 || (defined(__hpux) && defined(__HP_aCC)) \
 || (defined(linux) && defined(__INTEL_COMPILER) && defined(__ICC))
#  define BOOST_AUX_ANY_TYPE_ID_NAME
#  include <cstring>
# endif


namespace lunchbox
{
class Any
{
public:
    LUNCHBOX_API Any();

    template< typename ValueType >
    Any( const ValueType& value )
      : content( new holder< ValueType >( value ))
    {
    }

    LUNCHBOX_API Any( const Any& other );

    LUNCHBOX_API ~Any();

    LUNCHBOX_API Any& swap( Any& rhs );

    template< typename ValueType >
    Any& operator=( const ValueType& rhs )
    {
        Any( rhs ).swap( *this );
        return *this;
    }

    LUNCHBOX_API Any& operator=( Any rhs );

    LUNCHBOX_API bool empty() const;

    LUNCHBOX_API const std::type_info& type() const;

    LUNCHBOX_API bool operator == ( const Any& rhs ) const;

    bool operator != ( const Any& rhs ) const { return !(*this == rhs); }


    class placeholder
    {
    public:
        virtual ~placeholder() {}

        virtual bool operator == ( const placeholder& rhs ) const = 0;

        bool operator != ( const placeholder& rhs ) const
            { return !(*this == rhs); }

        virtual const std::type_info& type() const = 0;

        virtual placeholder* clone() const = 0;

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive & ar, const unsigned int version )
        {
        }
    };

    BOOST_SERIALIZATION_ASSUME_ABSTRACT(placeholder)

    template< typename ValueType >
    class holder : public placeholder
    {
    public:
        holder()
            : held()
        {
        }

        holder( const ValueType& value )
          : held( value )
        {
        }

        virtual const std::type_info& type() const
        {
            return typeid(ValueType);
        }

        virtual bool operator == ( const placeholder& rhs ) const
        {
            return held == static_cast< const holder& >( rhs ).held;
        }

        virtual placeholder* clone() const
        {
            return new holder( held );
        }

        ValueType held;

    private:
        holder& operator=( const holder& );

        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive & ar, const unsigned int version )
        {
            // serialize base class information
            ar & boost::serialization::base_object< placeholder >( *this );
            ar & held;
        }
    };

private:

    template< typename ValueType >
    friend ValueType* any_cast( Any* );

    template< typename ValueType >
    friend ValueType* unsafe_any_cast( Any* );

    friend class boost::serialization::access;
    template< class Archive >
    void serialize( Archive & ar, const unsigned int version )
    {
        ar & content;
    }

    boost::shared_ptr< placeholder > content;
};

class bad_any_cast : public std::bad_cast
{
public:
    LUNCHBOX_API bad_any_cast( const std::string& from, const std::string& to );

    virtual const char * what() const throw() { return data; }

private:
    char data[256];
};

template< typename ValueType >
ValueType* any_cast( Any* operand )
{
    return operand &&
#ifdef BOOST_AUX_ANY_TYPE_ID_NAME
        std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
        operand->type() == typeid(ValueType)
#endif
        ? &static_cast<Any::holder<ValueType> *>(operand->content.get())->held
        : 0;
}

template< typename ValueType >
inline const ValueType* any_cast( const Any* operand )
{
    return any_cast<ValueType>(const_cast<Any *>(operand));
}

template< typename ValueType >
ValueType any_cast( Any& operand )
{
    typedef typename boost::remove_reference< ValueType >::type nonref;

    nonref* result = any_cast< nonref >( &operand );
    if( !result )
        boost::throw_exception(
                      bad_any_cast( demangleTypeID( operand.type().name( )),
                                    demangleTypeID( typeid( nonref ).name( ))));
    return *result;
}

template< typename ValueType >
inline ValueType any_cast( const Any& operand )
{
    typedef typename boost::remove_reference< ValueType >::type nonref;

    return any_cast< const nonref& >( const_cast< Any& >( operand ));
}

// Note: The "unsafe" versions of any_cast are not part of the
// public interface and may be removed at any time. They are
// required where we know what type is stored in the any and can't
// use typeid() comparison, e.g., when our types may travel across
// different shared libraries.
template< typename ValueType >
inline ValueType* unsafe_any_cast( Any* operand )
{
    return &static_cast< Any::holder< ValueType >* >(
                             operand->content.get( ))->held;
}

template< typename ValueType >
inline const ValueType* unsafe_any_cast( const Any* operand )
{
    return unsafe_any_cast< ValueType >( const_cast< Any* > ( operand ));
}

template< typename ValueType >
ValueType unsafe_any_cast( Any& operand )
{
    typedef typename boost::remove_reference< ValueType >::type nonref;
    return *unsafe_any_cast< nonref >( &operand );
}

template< typename ValueType >
ValueType unsafe_any_cast( const Any& operand )
{
   typedef typename boost::remove_reference< ValueType >::type nonref;
   return unsafe_any_cast< const nonref& >( const_cast< Any& >( operand ));
}

}

#endif
