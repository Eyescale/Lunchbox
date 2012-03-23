
/* Copyright (c) 2011-2012, EFPL/Blue Brain Project
 *                     Stefan Eilemann <stefan.eilemann@epfl.ch> 
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

#ifndef DASH_DETAIL_ATTRIBUTE_H
#define DASH_DETAIL_ATTRIBUTE_H

#include "ContextPtr.h" // member (Attributes)
#include "types.h"
#include <lunchbox/referenced.h> // base class

namespace dash
{
namespace detail
{

/** The attribute implementation. */
class Attribute : public lunchbox::Referenced, public lunchbox::NonCopyable
{
public:
    explicit Attribute( dash::Attribute* attribute );
    explicit Attribute( dash::Attribute* attribute, AttributePtr from );
    ~Attribute();

    Attribute& operator = ( const Attribute& from );

    bool operator == ( const Attribute& rhs ) const;
    bool operator != ( const Attribute& rhs ) const { return !(*this == rhs); }

    void orphan() { attribute_ = 0; } //!< Released by parent dash::Attribute

    void map( const Context& from, const Context& to );
    void unmap( Context& context );

    void set( const detail::Any& value ) { *value_ = value; }
    detail::Any& get() { return *value_; }
    const detail::Any& get() const { return *value_; }

    dash::Attribute* getAttribute() { return attribute_; }
    const dash::Attribute* getAttribute() const { return attribute_; }

    void apply( const Change& change );

private:
    SERIALIZABLE()

    dash::Attribute* attribute_;
    AnyCtxPtr value_;

    Attribute();
    void notifyValueChanged_( Context& context, AnyCtxPtr::Value value );
};


template< class Archive >
inline void Attribute::save( Archive& ar, const unsigned int version ) const
{
    ar << *value_;
}

template< class Archive >
inline void Attribute::load( Archive& ar, const unsigned int version )
{
    ar >> *value_;
}

}
}

#endif // DASH_DETAIL_ATTRIBUTE_H
