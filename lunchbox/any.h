
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

/**
 * A class which can hold instances of any type.
 *
 * This class is based on boost.any with the extension to support serialization
 * if the ValueType supports boost.serialization.
 *
 * Example: @include tests/any.cpp
 */
class Any
{
public:
    /** @name Construction/Destruction */
    //@{
    /** Construct a new, empty Any. @version 1.5.0 */
    LUNCHBOX_API Any();

    /** Construct a new Any with the given value. @version 1.5.0 */
    template< typename ValueType >
    Any( const ValueType& value )
      : content( new holder< ValueType >( value ))
    {
    }

    /** Copy-construct a new Any with copying content of other. @version 1.5.0*/
    LUNCHBOX_API Any( const Any& other );

    /** Destruct this Any. @version 1.5.0 */
    LUNCHBOX_API ~Any();
    //@}

    /** @name Modifiers */
    //@{
    /** Exchange the content of this and rhs. @version 1.5.0 */
    LUNCHBOX_API Any& swap( Any& rhs );

    /** Assign a new value to this Any. @version 1.5.0 */
    template< typename ValueType >
    Any& operator=( const ValueType& rhs )
    {
        Any( rhs ).swap( *this );
        return *this;
    }

    /** Exchange the content of this and rhs. @version 1.5.0 */
    LUNCHBOX_API Any& operator=( Any rhs );
    //@}

    /** @name Queries */
    //@{
    /** @return true if this Any is not holding a value. @version 1.5.0 */
    LUNCHBOX_API bool empty() const;

    /**
     * @return the typeid of the contained value if non-empty, otherwise
     * typeid(void).
     * @version 1.5.0
     */
    LUNCHBOX_API const std::type_info& type() const;

    /**
     * @return true if this and rhs are empty or if their values are equal.
     * @version 1.5.0
     */
    LUNCHBOX_API bool operator == ( const Any& rhs ) const;

    /**
     * @return true if the value from this and rhs are not equal.
     * @version 1.5.0
     */
    bool operator != ( const Any& rhs ) const { return !(*this == rhs); }
    //@}


    /** @cond IGNORE */
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
        void serialize( Archive&, const unsigned int ) {}
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

        explicit holder( const ValueType& value )
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
        void serialize( Archive & ar, const unsigned int /*version*/ )
        {
            // serialize base class information
            ar & boost::serialization::base_object< placeholder >( *this );
            ar & held;
        }
    };
    /** @endcond */

private:
    template< typename ValueType >
    friend ValueType* any_cast( Any* );

    template< typename ValueType >
    friend ValueType* unsafe_any_cast( Any* );

    friend class boost::serialization::access;
    template< class Archive >
    void serialize( Archive & ar, const unsigned int /*version*/ )
    {
        ar & content;
    }

    boost::shared_ptr< placeholder > content;
};

/** A specialization for exceptions thrown by an unsuccessful any_cast. */
class bad_any_cast : public std::bad_cast
{
public:
    LUNCHBOX_API bad_any_cast( const std::string& from, const std::string& to );

    virtual const char* what() const throw() { return data; }

private:
    char data[256];
};

/**
 * Retrieve the value stored in an Any including type checking.
 *
 * @return the value stored in the given Any, 0 if types are not matching
 * @version 1.5.0
 */
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

/**
 * Retrieve the value stored in an Any including type checking.
 *
 * @return the value stored in the given Any, 0 if types are not matching
 * @version 1.5.0
 */
template< typename ValueType >
inline const ValueType* any_cast( const Any* operand )
{
    return any_cast<ValueType>(const_cast<Any *>(operand));
}

/**
 * Retrieve the value stored in an Any including type checking.
 *
 * @return the value stored in the given Any
 * @throws bad_any_cast if types are not matching
 * @version 1.5.0
 */
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

/**
 * Retrieve the value stored in an Any including type checking.
 *
 * @return the value stored in the given Any
 * @throws bad_any_cast if types are not matching
 * @version 1.5.0
 */
template< typename ValueType >
inline ValueType any_cast( const Any& operand )
{
    typedef typename boost::remove_reference< ValueType >::type nonref;

    return any_cast< const nonref& >( const_cast< Any& >( operand ));
}

/**
 * Retrieve the value stored in an Any without type checking.
 *
 * @return the value stored in the given Any
 * @version 1.5.0
 */
template< typename ValueType >
inline ValueType* unsafe_any_cast( Any* operand )
{
    return &static_cast< Any::holder< ValueType >* >(
                             operand->content.get( ))->held;
}

/**
 * Retrieve the value stored in an Any without type checking.
 *
 * @return the value stored in the given Any
 * @version 1.5.0
 */
template< typename ValueType >
inline const ValueType* unsafe_any_cast( const Any* operand )
{
    return unsafe_any_cast< ValueType >( const_cast< Any* > ( operand ));
}

/**
 * Retrieve the value stored in an Any without type checking.
 *
 * @return the value stored in the given Any
 * @version 1.5.0
 */
template< typename ValueType >
ValueType unsafe_any_cast( Any& operand )
{
    typedef typename boost::remove_reference< ValueType >::type nonref;
    return *unsafe_any_cast< nonref >( &operand );
}

/**
 * Retrieve the value stored in an Any without type checking.
 *
 * @return the value stored in the given Any
 * @version 1.5.0
 */
template< typename ValueType >
ValueType unsafe_any_cast( const Any& operand )
{
   typedef typename boost::remove_reference< ValueType >::type nonref;
   return unsafe_any_cast< const nonref& >( const_cast< Any& >( operand ));
}

}

#endif
