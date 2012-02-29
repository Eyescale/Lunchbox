
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_CHANGE_H
#define DASH_DETAIL_CHANGE_H

#include "types.h"
#include <dash/api.h>
#include <iostream>

namespace dash
{
namespace detail
{

/** A modification in a context. */
struct Change
{
    enum Type
    {
        NONE,
        NODE_INSERT,
        NODE_ERASE,
        ATTRIBUTE_INSERT,
        ATTRIBUTE_ERASE,
        ATTRIBUTE_CHANGED
    };

    Change() : type( NONE ) {}
    Change( const Type t, NodePtr n, dash::NodePtr c );
    Change( const Type t, NodePtr n, dash::AttributePtr a );
    Change( dash::AttributePtr a, std::tr1::shared_ptr<boost::any> value );

    Type type;
    NodePtr node;
    dash::NodePtr child;

    dash::AttributePtr attribute;
    std::tr1::shared_ptr< boost::any > value;

    std::tr1::shared_ptr< dash::Context > context;
};

DASH_API std::ostream& operator << ( std::ostream& os, const Change& change );
DASH_API std::ostream& operator << ( std::ostream& os, const Change::Type& type);

}
}

#endif // DASH_DETAIL_CHANGE_H
