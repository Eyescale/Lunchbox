
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_CONTEXTPTR_H
#define DASH_DETAIL_CONTEXTPTR_H

#include "Context.h" // used inline
#include <dash/Vector.h> // member
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
    typedef std::tr1::shared_ptr< T > Value;

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
            EQASSERTINFO( values_.size() > context.getSlot() &&
                          values_[ context.getSlot() ].get(),
                          "Access of unmapped object in context " <<
                          context.getSlot( ));
            return *values_[ context.getSlot() ];
        }

    T& get( Context& context = Context::getCurrent( ))
        {
            EQASSERTINFO( values_.size() > context.getSlot() &&
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

private:
    typedef Vector< Value > Values;
    Values values_;
    changed_t cb_;

    ContextPtr( const ContextPtr& from );
    ContextPtr& operator = ( const ContextPtr& from );
};

}
}

#endif // DASH_DETAIL_CONTEXTPTR_H
