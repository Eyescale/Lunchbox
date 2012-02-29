
// Copyright (c) 2011 EPFL/BBP. All rights reserved.
// Author: Stefan Eilemann

#ifndef DASH_DETAIL_NODE_H
#define DASH_DETAIL_NODE_H

#include "ContextPtr.h" // member (Nodes)
#include "ContextPtr.h" // member (Nodes)
#include <dash/Attribute.h> // AttributePtr return value
#include <co/base/referenced.h> // base class

namespace dash
{
namespace detail
{

/** The node implementation. */
class Node : public co::base::Referenced, public co::base::NonCopyable
{
public:
    explicit Node( dash::Node* node );
    explicit Node( dash::Node* node, NodePtr from );
    ~Node();

    Node& operator = ( const Node& from );

    /** Set up a new slot for the node. */
    void map( const Context& from, const Context& to );
    /** Release references held by the context. */
    void unmap( Context& context );
    /** @return true if the Node is mapped to the context. */
    bool isMapped( const Context& context = Context::getCurrent( )) const;

    void orphan() { node_ = 0; } //!< Released by parent dash::Node

    /** @name Hierarchy */
    //@{
    bool hasParents() const { return !parents_->empty(); }
    size_t getNParents() const { return parents_->size(); }

    size_t getNChildren() const { return children_->size(); }
    dash::NodePtr getChild( const size_t i )
        { return (*static_cast< const Node * >( this )->children_)[ i ]; }
    dash::NodeConstPtr getChild( const size_t i ) const
        { return (*children_)[ i ]; }

    void insert( dash::NodePtr child );
    bool erase( dash::NodePtr child );
    //@}

    /** @name Attributes */
    //@{
    size_t getNAttributes() const { return attributes_->size(); }
    dash::AttributePtr getAttribute( const size_t i )
        { return (*static_cast< const Node * >( this )->attributes_)[ i ]; }
    dash::AttributeConstPtr getAttribute( const size_t i ) const
        { return (*attributes_)[ i ]; }

    void insert( dash::AttributePtr attribute );
    bool erase( dash::AttributePtr attribute );
    //@}

    dash::Node* getNode() { return node_; }
    const dash::Node* getNode() const { return node_; }

private:
    friend int test::main( int argc, char **argv );
    dash::Node* node_;
    ParentsCtxPtr parents_;
    ChildrenCtxPtr children_;
    AttributesCtxPtr attributes_;

    Node();
};

}
}

#endif // DASH_DETAIL_NODE_H
