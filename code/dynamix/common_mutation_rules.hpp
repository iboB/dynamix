// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "globals.hpp"
#include "mutation_rule_info.hpp"
#include "type_mutation.hpp"

namespace dynamix::util::builder_literals {

// C++23: all this can be solved much better with static operator()

namespace priv {
template <typename Primary, typename Dependent>
error_return_t apply_basic_mixin_dep(dnmx_type_mutation_handle mutation, uintptr_t) {
    auto mut = type_mutation::from_c_handle(mutation);
    if (mut->has<Primary>()) {
        mut->add_if_lacking<Dependent>();
    }
    return result_success;
}
}

struct common_mutation_rule {
    mutation_rule_info::apply_func apply;
    bool makes_dependency;
};

template <typename Primary>
struct attaches_to {
    static inline constexpr std::string_view name = "attaches_to";
    template <typename Dependent>
    static common_mutation_rule get_rule_for() {
        return {
            &priv::apply_basic_mixin_dep<Primary, Dependent>,
            true
        };
    }
};

template <typename Dependent>
struct also_adds {
    static inline constexpr std::string_view name = "also_adds";
    template <typename Primary>
    static common_mutation_rule get_rule_for() {
        return {
            &priv::apply_basic_mixin_dep<Primary, Dependent>,
            false
        };
    }
};

// It's possible to add a pointer returning func in case no perks are added?
//  template <typename M>
//  static mutation_rule_info* get_rule_for() { static mutation_rule_info ret{...}; return &ret; }
// ... but do we need it?

}
