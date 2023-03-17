// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_mutate_ops.hpp"
#include "type_mutation.hpp"

namespace dynamix {
template <typename Mixin>
struct common_mixin_init : public object_mutate_op {
    common_mixin_init() {
        this->affected_info = &g::get_mixin_info<Mixin>();
    }

    virtual bool overrides_init() const noexcept final override { return true; }

    // compatibility with mutate ops
    void apply_to_type_mutation(type_mutation& mut) { mut.add(*this->affected_info); }
};
}