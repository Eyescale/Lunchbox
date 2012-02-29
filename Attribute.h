
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_ATTRIBUTE_H
#define DASH_DETAIL_ATTRIBUTE_H

#include "ContextPtr.h" // member (Attributes)
#include "types.h"
#include <co/base/referenced.h> // base class

namespace dash
{
namespace detail
{

/** The attribute implementation. */
class Attribute : public co::base::Referenced, public co::base::NonCopyable
{
public:
    explicit Attribute( dash::Attribute* attribute );
    explicit Attribute( dash::Attribute* attribute, AttributePtr from );
    ~Attribute();

    Attribute& operator = ( const Attribute& from );

    void orphan() { attribute_ = 0; } //!< Released by parent dash::Attribute

    void map( const Context& from, const Context& to );
    void unmap( Context& context );

    void set( const boost::any& value ) { *value_ = value; }
    boost::any& get() { return *value_; }
    const boost::any& get() const { return *value_; }

    dash::Attribute* getAttribute() { return attribute_; }
    const dash::Attribute* getAttribute() const { return attribute_; }

    void apply( const Change& change );

private:
    dash::Attribute* attribute_;
    AnyCtxPtr value_;

    Attribute();
    void notifyValueChanged_( Context& context, AnyCtxPtr::Value value );
};

}
}

#endif // DASH_DETAIL_ATTRIBUTE_H
