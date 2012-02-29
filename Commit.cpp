
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#include "Commit.h"

#include "Attribute.h"
#include "Change.h"
#include "Node.h"

#include <dash/Context.h>
#include <dash/Node.h>

namespace dash
{
namespace detail
{

Commit::Commit()
     : changes_( new Changes )
{
}

Commit::~Commit()
{
}

void Commit::add( const Change& change )
{
    changes_->push_back( change );
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
                  change.context->map( change.child,
                                       dash::Context::getCurrent( ));
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
                  change.context->map( change.attribute,
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
