// Copyright (c) 2020-2023 by the Zeek Project. See LICENSE for details.

#pragma once

#include <utility>

#include <hilti/ast/meta.h>
#include <hilti/ast/node.h>
#include <hilti/ast/statement.h>

namespace spicy::statement {
class Reject : public hilti::NodeBase, public hilti::trait::isStatement {
public:
    Reject(hilti::Meta m = hilti::Meta()) : hilti::NodeBase(std::move(m)) {}

    friend bool operator==(const Reject&, const Reject&) { return false; }

    // Statement interface.
    auto isEqual(const hilti::Statement& other) const { return hilti::node::isEqual(this, other); }

    // Node interface.
    auto properties() const { return hilti::node::Properties{}; }
};
} // namespace spicy::statement
