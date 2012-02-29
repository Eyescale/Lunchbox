
/* Copyright (c) 2011, EFPL/Blue Brain Project
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
#include <co/base/referenced.h> // base class

namespace dash
{
namespace detail
{

/** The attribute implementation. */
class Attribute : public co::base::Referenced, public co::base::NonCopyable
{
public:
    explicit Attribute( dash::Attribute* attribute );
    explicit Attribute( dash::Attribute* attribute, AttributePtr from );
    ~Attribute();

    Attribute& operator = ( const Attribute& from );

    void orphan() { attribute_ = 0; } //!< Released by parent dash::Attribute

    void map( const Context& from, const Context& to );
    void unmap( Context& context );

    void set( const boost::any& value ) { *value_ = value; }
    boost::any& get() { return *value_; }
    const boost::any& get() const { return *value_; }

    dash::Attribute* getAttribute() { return attribute_; }
    const dash::Attribute* getAttribute() const { return attribute_; }

    void apply( const Change& change );

private:
    dash::Attribute* attribute_;
    AnyCtxPtr value_;

    Attribute();
    void notifyValueChanged_( Context& context, AnyCtxPtr::Value value );
};

}
}

#endif // DASH_DETAIL_ATTRIBUTE_H
