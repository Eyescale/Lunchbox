
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

#ifndef DASH_DETAIL_CONTEXT_H
#define DASH_DETAIL_CONTEXT_H

#include "types.h"

#include <dash/api.h>
#include <dash/Serializable.h>
#include <lunchbox/lfVector.h> // member
#include <lunchbox/types.h>

namespace dash
{
namespace detail
{

/** Internal implementation of the dash::Context */
class Context
{
public:
    Context(); //!< Create a new context
    ~Context();

    DASH_API static Context& getCurrent();

    static size_t getNumSlots();
    size_t getSlot() const { return slot_; }

    void map( dash::NodePtr node, const Context& to );
    void unmap( dash::NodePtr node );

    void map( AttributePtr attribute, const Context& to );
    void unmap( AttributePtr attribute );

    CommitPtr getCommit();
    CommitConstPtr getCommit() const;

    void addChange( const Change& change );
    dash::Commit commit();
    void apply( CommitConstPtr commit );

private:
    SERIALIZABLE()

    friend int test::main( int argc, char **argv );

    size_t slot_; //!< lookup index for data of this Context
    dash::Commit* commit_; //!< pending changes

    Context( const Context& from ); // disable copy
    Context& operator = ( const Context& from ); // disable assignment

    EQ_TS_VAR( thread_ );
};

template< class Archive >
inline void Context::save( Archive& ar, const unsigned int version ) const
{
    ar << slot_;
}

template< class Archive >
inline void Context::load( Archive& ar, const unsigned int version )
{
    ar >> slot_;
}

}
}

#endif // DASH_DETAIL_CONTEXT_H
