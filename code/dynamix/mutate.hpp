// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mutate_ops.hpp"
#include "mutate_to.hpp"

namespace dynamix {
template <typename... Ops>
void mutate(object& obj, Ops&&... ops) {
    type_mutation type_mut(obj.get_type());

    (ops.apply_to_type_mutation(type_mut), ...);

    auto& type = obj.get_domain().get_type(std::move(type_mut));

    mutate_to(obj, type, std::forward<Ops>(ops)...);
}
}
