
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

#ifndef DASH_DETAIL_CONTEXTPTR_H
#define DASH_DETAIL_CONTEXTPTR_H

#include "Context.h" // used inline
#include <lunchbox/lfVector.h> // member
#include <boost/function.hpp> // used inline

namespace dash
{
namespace detail
{

/** The dash context-aware pointer for multi-buffered data access. */
template< class T > class ContextPtr
{
public:
    /** The value pointer. */
    typedef boost::shared_ptr< T > Value;

    /** The type of the callback function invoked after a copy-on-write. */
    typedef boost::function< void( Context&, Value ) > changed_t;

    ContextPtr()
            : values_( Context::getCurrent().getNumSlots(), Value( ))
        {}
#if 0
    ContextPtr( const T& item )
            : values_( Context::getCurrent().getNumSlots(), Value(new T( item )))
        {}
#endif

    ~ContextPtr() {}

    void setChangedCB( const changed_t& callback ) { cb_ = callback; }

    const T& get( const Context& context = Context::getCurrent( )) const
        {
            LBASSERTINFO( values_.size() > context.getSlot() &&
                          values_[ context.getSlot() ].get(),
                          "Access of unmapped object in context " <<
                          context.getSlot( ));
            return *values_[ context.getSlot() ];
        }

    T& get( Context& context = Context::getCurrent( ))
        {
            LBASSERTINFO( values_.size() > context.getSlot() &&
                          values_[ context.getSlot() ].get(),
                          "Access to unmapped object in context " <<
                          context.getSlot( ));

            Value& value = values_[ context.getSlot( )];
            if( !value.unique( ))
            {
                value = Value( new T( *value ));
                if( !cb_.empty( ))
                    cb_( context, value );
            }

            return *value;
        }

    const T* operator->() const { return &get(); }
    T* operator->() { return &get(); }

    const T& operator*() const { return get(); }
    T& operator*() { return get(); }

    /** Set up a new slot for the to context using the from context data. */
    void map( const Context& from, const Context& to )
        {
            const int32_t toSlot = to.getSlot();
            values_.expand( toSlot + 1 );
            values_[ toSlot ] = values_[ from.getSlot() ];
        }

    /** Clear the slot for the given context. */
    void unmap( const Context& context )
        { values_[ context.getSlot() ].reset(); }

    /** @return true if the pointer has been mapped to the given context. */
    bool isMapped( const Context& context ) const
        { return values_[ context.getSlot() ]; }

    /** Ensure the existence of a slot for the context, using a default value. */
    void setup( const Context& context = Context::getCurrent(),
                Value defaultValue = Value( new T ))
        {
            const int32_t slot = context.getSlot();
            values_.expand( slot + 1 );
            if( !values_[ slot ].get( ))
                values_[ slot ] = defaultValue;
        }

    void apply( Value value, Context& context = Context::getCurrent( ))
        {
            const int32_t slot = context.getSlot();
            if( value == values_[ slot ] )
                return;

            values_[ slot ] = value;
            if( cb_ )
                cb_( context, value );
        }

    /** @internal */
    void set( Value value, Context& context = Context::getCurrent( ))
        {
            const int32_t slot = context.getSlot();
            values_.expand( slot + 1 );
            values_[ slot ] = value;
        }

private:
    typedef LFVector< Value > Values;
    Values values_;
    changed_t cb_;

    ContextPtr( const ContextPtr& from );
    ContextPtr& operator = ( const ContextPtr& from );
};

}
}

#endif // DASH_DETAIL_CONTEXTPTR_H
