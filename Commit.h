
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_COMMIT_H
#define DASH_DETAIL_COMMIT_H

#include "types.h"

#include <dash/Vector.h> // member
#include <co/base/types.h>

namespace dash
{
namespace detail
{

/** Internal implementation of the dash::Commit */
class Commit
{
public:
    Commit();
    ~Commit();

    bool empty() const { return changes_->empty(); }

    void add( const Change& change );
    void apply() const;

private:
    friend int test::main( int argc, char **argv );
    friend std::ostream& operator << ( std::ostream& os, const Commit& commit );

    ChangesPtr changes_;

    Commit( const Commit& from ); // disable copy
    Commit& operator = ( const Commit& from ); // disable assignment
};

inline std::ostream& operator << ( std::ostream& os, const Commit& commit )
{
    return os << commit.changes_;
}

}
}

#endif // DASH_DETAIL_COMMIT_H
