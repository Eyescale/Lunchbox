
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

#include "Commit.h"

#include "Attribute.h"
#include "Change.h"
#include "Node.h"

#include <dash/Node.h>

namespace dash
{
namespace detail
{

Commit::Commit()
     : changes_( new Changes )
     , context_()
{
}

Commit::~Commit()
{
}

bool Commit::operator == ( const Commit& rhs ) const
{
    if( this == &rhs )
        return true;

    if( changes_->size() != rhs.changes_->size( ))
        return false;

    ChangesCIter it = changes_->begin();
    ChangesCIter rhsIt = rhs.changes_->begin();
    for( ; it != changes_->end() && rhsIt != rhs.changes_->end();
         ++it, ++rhsIt )
    {
        if( *it != *rhsIt )
            return false;
    }

    return true;
}

void Commit::add( const Change& change )
{
    EQASSERT( Context::getNumSlots() > 1 );

    if( !context_ )
        context_.reset( new dash::Context );

    if( change.type == Change::NODE_INSERT )
        dash::Context::getCurrent().map( change.child, *context_ );
    else if( change.type == Change::ATTRIBUTE_INSERT )
        dash::Context::getCurrent().map( change.attribute, *context_ );

    changes_->push_back( change );
    const_cast<Change&>(change).commit = this;
}

void Commit::apply() const
{
    for( ChangesIter i = changes_->begin(); i != changes_->end(); ++i )
    {
        Change& change = *i;
        switch( change.type )
        {
          case Change::NODE_INSERT:
              if( change.node->isMapped( ))
              {
                  context_->map( change.child, dash::Context::getCurrent( ));
                  change.node->insert( change.child );
              }
              else
                  EQINFO << "Ignoring Node::insert change, parent not mapped"
                         << std::endl;
              break;
          case Change::NODE_ERASE:
              if( !change.node->isMapped( ))
                  break;
              change.node->erase( change.child );
              break;

          case Change::ATTRIBUTE_INSERT:
              if( change.node->isMapped( ))
              {
                  context_->map( change.attribute,
                                 dash::Context::getCurrent( ));
                  change.node->insert( change.attribute );
              }
              else
                  EQINFO << "Ignoring Node::insert change, parent not mapped"
                         << std::endl;
              break;

          case Change::ATTRIBUTE_ERASE:
              if( !change.node->isMapped( ))
                  break;
              change.node->erase( change.attribute );
              break;

          case Change::ATTRIBUTE_CHANGED:
              change.attribute->getImpl()->apply( change );
              break;

          default:
              EQINFO << change << std::endl;
              EQUNIMPLEMENTED;
        }
    }
}

}
}
