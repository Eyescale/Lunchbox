
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

#include "Change.h"

#include "Attribute.h"
#include "Node.h"

#include <dash/Context.h>
#include <dash/Node.h>

namespace dash
{
namespace detail
{
namespace
{
struct NodeContextDeleter
{
    NodeContextDeleter( dash::NodePtr n ) : node( n ) {}
    void operator()( dash::Context* context ) const
        {
            context->unmap( node );
            delete context;
        }

    dash::NodePtr node;
};

struct AttributeContextDeleter
{
    AttributeContextDeleter( dash::AttributePtr a ) : attribute( a ) {}
    void operator()( dash::Context* context ) const
        {
            context->unmap( attribute );
            delete context;
        }

    dash::AttributePtr attribute;
};
}

Change::Change( const Type t, NodePtr p, dash::NodePtr c )
        : type( t )
        , node( p )
        , child( c )
{
    // OPT: Single context does not record changes
    if( Context::getNumSlots() > 1 && t == NODE_INSERT )
    {
        context = std::tr1::shared_ptr< dash::Context >(
            new dash::Context, NodeContextDeleter( child ));
        dash::Context::getCurrent().map( child, *context );
    }
}

Change::Change( const Type t, NodePtr p, dash::AttributePtr a )
        : type( t )
        , node( p )
        , attribute( a )
{
    // OPT: Single context does not record changes
    if( Context::getNumSlots() > 1 && t == ATTRIBUTE_INSERT )
    {
        context = std::tr1::shared_ptr< dash::Context >(
            new dash::Context, AttributeContextDeleter( a ));
        dash::Context::getCurrent().map( a, *context );
    }
}

Change::Change( dash::AttributePtr a, std::tr1::shared_ptr< detail::Any > v )
        : type( ATTRIBUTE_CHANGED )
        , attribute( a )
        , value( v )
{}

std::ostream& operator << ( std::ostream& os, const Change& change )
{
    return os << "Change " << change.type;
}

std::ostream& operator << ( std::ostream& os, const Change::Type& type )
{
    return os << ( type == Change::NONE ? "NONE" :
                   type == Change::NODE_INSERT ? "node insert" :
                   type == Change::NODE_ERASE ? "node erase" :
                   type == Change::ATTRIBUTE_INSERT ? "attribute insert" :
                   type == Change::ATTRIBUTE_ERASE ? "attribute erase" :
                   type == Change::ATTRIBUTE_CHANGED  ? "attribute change" :
                                                        "ERROR" );
}

}
}
