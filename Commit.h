
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
