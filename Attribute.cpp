
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

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
    value_.get( context ) = boost::any();
}

void Attribute::apply( const Change& change )
{
    EQASSERT( change.type == Change::ATTRIBUTE_CHANGED );
    value_.apply( change.value );
}

}
}
