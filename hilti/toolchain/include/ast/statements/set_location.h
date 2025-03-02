// Copyright (c) 2020-2023 by the Zeek Project. See LICENSE for details.

#pragma once

#include <utility>

#include <hilti/ast/expression.h>
#include <hilti/ast/meta.h>
#include <hilti/ast/node.h>
#include <hilti/ast/statement.h>

namespace hilti::statement {
class SetLocation : public hilti::NodeBase, public hilti::trait::isStatement {
public:
    SetLocation(hilti::Expression e, hilti::Meta m = hilti::Meta())
        : hilti::NodeBase(hilti::nodes(std::move(e)), std::move(m)) {}

    auto expression() const { return children()[0].tryAs<hilti::Expression>(); }

    friend bool operator==(const SetLocation&, const SetLocation&) { return false; }

    // Statement interface.
    auto isEqual(const hilti::Statement& other) const { return hilti::node::isEqual(this, other); }

    // Node interface.
    auto properties() const { return hilti::node::Properties{}; }
};
} // namespace hilti::statement
