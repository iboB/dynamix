// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mutate_ops.hpp"
#include "mutate_to.hpp"

namespace dynamix {
// regular mutate with ops as arguments
template <typename... Ops>
void mutate(object& obj, Ops&&... ops) {
    type_mutation type_mut(obj.get_type());

    (ops.apply_to_type_mutation(type_mut), ...);

    auto& type = obj.get_domain().get_type(std::move(type_mut));

    mutate_to(obj, type, std::forward<Ops>(ops)...);
}

// v1 comaptible mutate which only works with mixins with default init functions
// not extensible
class gradual_mutation {
    type_mutation m_type_mut;
    object& m_obj;
public:
    gradual_mutation(object& obj)
        : m_type_mut(obj.get_type())
        , m_obj(obj)
    {}

    using self_t = gradual_mutation;

    template <typename Mixin>
    self_t& add() {
        m_type_mut.add<Mixin>();
        return *this;
    }
};
gradual_mutation mutate(object& obj) {
    return gradual_mutation(obj);
}
}
