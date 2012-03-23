
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

#ifndef DASH_DETAIL_CHANGE_H
#define DASH_DETAIL_CHANGE_H

#include "types.h"
#include "Node.h"
#include <dash/api.h>
#include <dash/Serializable.h>
#include <dash/Context.h>
#include "Commit.h"
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
    Change( dash::AttributePtr a, boost::shared_ptr<detail::Any> value );

    bool operator == ( const Change& rhs ) const;
    bool operator != ( const Change& rhs ) const { return !(*this == rhs); }

    Type type;
    NodePtr node;
    dash::NodePtr child;

    dash::AttributePtr attribute;
    boost::shared_ptr< detail::Any > value;
    Commit* commit;

private:
    SERIALIZABLE()
};

DASH_API std::ostream& operator << ( std::ostream& os, const Change& change );
DASH_API std::ostream& operator << ( std::ostream& os, const Change::Type& type);

template< class Archive >
inline void Change::save( Archive& ar, const unsigned int version ) const
{
    ar << type;
    ar << node;
    if( type == Change::NODE_INSERT )
    {
        //dash::Context& current = dash::Context::getCurrent();
        //commit->context_->setCurrent();
        ar << child;
        //current.setCurrent();
    }
    else if( type == Change::ATTRIBUTE_INSERT )
    {
        //dash::Context& current = dash::Context::getCurrent();
        //commit->context_->setCurrent();
        ar << attribute;
        //current.setCurrent();
    }
    ar << value;
}

template< class Archive >
inline void Change::load( Archive& ar, const unsigned int version )
{
    ar >> type;
    ar >> node;
    if( type == Change::NODE_INSERT )
    {
        //dash::Context& current = dash::Context::getCurrent();
        //commit->context_->setCurrent();
        ar >> child;
        //current.setCurrent();
    }
    else if( type == Change::ATTRIBUTE_INSERT )
    {
        //dash::Context& current = dash::Context::getCurrent();
        //commit->context_->setCurrent();
        ar >> attribute;
        //current.setCurrent();
    }
    ar >> value;
}

}
}

#endif // DASH_DETAIL_CHANGE_H
