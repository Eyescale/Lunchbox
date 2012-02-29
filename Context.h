
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_CONTEXT_H
#define DASH_DETAIL_CONTEXT_H

#include "types.h"

#include <dash/Vector.h> // member
#include <co/base/types.h>

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

    static Context& getCurrent();

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
    friend int test::main( int argc, char **argv );

    const size_t slot_; //!< lookup index for data of this Context
    dash::Commit* commit_; //!< pending changes

    Context( const Context& from ); // disable copy
    Context& operator = ( const Context& from ); // disable assignment

    EQ_TS_VAR( thread_ );
};

}
}

#endif // DASH_DETAIL_CONTEXT_H
