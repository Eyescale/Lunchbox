
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

#include "Node.h"

#include "Attribute.h"
#include "Change.h"
#include "Context.h"
#include "types.h"
#include "../Node.h"

#include <algorithm>

namespace dash
{
namespace detail
{

Node::Node( dash::Node* node )
        : node_( node )
{
    parents_.setup();
    children_.setup();
    attributes_.setup();
}

Node::Node( dash::Node* node, NodePtr from )
        : node_( node )
{
    for( ChildrenCIter i = from->children_->begin();
         i != from->children_->end(); ++i )
    {
        insert( *i );
    }
    for( AttributesCIter i = from->attributes_->begin();
         i != from->attributes_->end(); ++i )
    {
        insert( *i );
    }
}

Node::~Node()
{
    EQASSERT( node_ == 0 );
    while( !children_->empty( ))
    {
        // Don't use erase, generates Change
        dash::NodePtr child = children_->back();
        NodePtr childImpl = child->getImpl();
        ParentsCtxPtr& parents = childImpl->parents_;
        ParentsIter j = std::find( parents->begin(), parents->end(), this );
        EQASSERT( j != parents->end( ));

        parents->erase( j );
        children_->pop_back();
    }

    attributes_->clear();
}

Node& Node::operator = ( const Node& from )
{
    if( this == &from )
        return *this;

    EQUNIMPLEMENTED;
    return *this;
}

bool Node::operator == ( const Node& rhs ) const
{
    if( this == &rhs )
        return true;

    if( parents_->size() != rhs.parents_->size() ||
        children_->size() != rhs.children_->size() ||
        attributes_->size() != rhs.attributes_->size( ))
    {
        return false;
    }

    Attributes::const_iterator it = attributes_->begin();
    Attributes::const_iterator rhsIt = rhs.attributes_->begin();
    for( ; it != attributes_->end() && rhsIt != rhs.attributes_->end();
         ++it, ++rhsIt )
    {
        if( **it != **rhsIt )
            return false;
    }

    return true;
}


void Node::map( const Context& from, const Context& to )
{
    parents_.map( from, to );
    children_.map( from, to );
    attributes_.map( from, to );
}

void Node::unmap( Context& context )
{
    parents_.unmap( context );
    children_.unmap( context );
    attributes_.unmap( context );
}

bool Node::isMapped( const Context& context ) const
{
    return parents_.isMapped( context );
}

void Node::insert( dash::NodePtr child )
{
    Change change( Change::NODE_INSERT, this, child );
    Context::getCurrent().addChange( change );
    children_->push_back( child );
    child->getImpl()->parents_->push_back( this );    
}

bool Node::erase( dash::NodePtr child )
{
    ChildrenIter i = std::find( children_->begin(), children_->end(), child);
    if( i == children_->end( ))
        return false;

    Change change( Change::NODE_ERASE, this, child );
    Context::getCurrent().addChange( change );

    NodePtr childImpl = child->getImpl();

    ParentsCtxPtr& parents = childImpl->parents_;
    ParentsIter j = std::find( parents->begin(), parents->end(), this );
    EQASSERT( j != parents->end( ));

    parents->erase( j );
    children_->erase( i );
    return true;
}

void Node::insert( dash::AttributePtr attribute )
{
    Change change( Change::ATTRIBUTE_INSERT, this, attribute );
    Context::getCurrent().addChange( change );
    attributes_->push_back( attribute );    
}

bool Node::erase( dash::AttributePtr attribute )
{
    AttributesIter i = std::find( attributes_->begin(), attributes_->end(),
                                  attribute );
    if( i == attributes_->end( ))
        return false;

    Change change( Change::ATTRIBUTE_ERASE, this, attribute );
    Context::getCurrent().addChange( change );

    attributes_->erase( i );
    return true;
}

}
}
