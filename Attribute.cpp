
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

#include "Attribute.h"

#include "types.h"
#include "Change.h"
#include "Node.h"

#include <dash/Attribute.h>
#include <dash/Node.h>

#include <boost/bind.hpp>

namespace dash
{
namespace detail
{

Attribute::Attribute( dash::Attribute* attribute )
        : attribute_( attribute )
{
    value_.setup();
    value_.setChangedCB(
        boost::bind( &Attribute::notifyValueChanged_, this, _1, _2 ));
}

Attribute::Attribute( dash::Attribute* attribute, AttributePtr from )
        : attribute_( attribute )
{
}

Attribute::~Attribute()
{
    EQASSERT( attribute_ == 0 );
}

Attribute& Attribute::operator = ( const Attribute& from )
{
    if( this == &from )
        return *this;

    EQUNIMPLEMENTED;
    return *this;
}

bool Attribute::operator == ( const Attribute& rhs ) const
{
    if( this == &rhs )
        return true;

    if( get().type() != rhs.get().type() )
        return false;

    EQUNIMPLEMENTED;
    return true;
}

void Attribute::notifyValueChanged_( Context& context, AnyCtxPtr::Value value )
{
    Change change( attribute_, value );
    context.addChange( change );
}

void Attribute::map( const Context& from, const Context& to )
{
    value_.map( from, to );
}

void Attribute::unmap( Context& context )
{
    value_.unmap( context );
}

void Attribute::apply( const Change& change )
{
    EQASSERT( change.type == Change::ATTRIBUTE_CHANGED );
    value_.apply( change.value );
}

}
}
