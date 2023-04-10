// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_mutate_ops.hpp"
#include "mutate_to_ops.hpp"
#include "object_mutation.hpp"

#include <algorithm>

namespace dynamix {
inline void mutate_to(object& obj, const type& type) {
    obj.reset_type(type);
}

template <typename... Ops>
void mutate_to(object& obj, const type& type, Ops&&... ops) {
    // here we need to:
    // * filter out ops that are not applicable for the object mutation which is to come
    // * sort the applicable ones by index in the new type
    // * apply
    // we could do it boost::mpl-style: by doint it in a tuple of ops
    // the code however will be very hard to follow and will compile for a really long time
    // instead we sacrifice a tiny bit of performance and do it in a more readable fashion
    // and help the the compiler a bit with this complicated task

    object_mutate_op* obj_mutates[] = {ops.to_obj_mutate(obj)...};
    auto obj_mutates_end = obj_mutates + sizeof...(Ops);

    // do multiple things reusing the remove_if loop
    obj_mutates_end = std::remove_if(obj_mutates, obj_mutates_end, [&type](object_mutate_op* op) {
        if (!op) return true; // null (non add-mixin op)
        if (!op->overrides_init()) return true; // valid op, which doesn't override init, so we don't need it
        auto index = type.index_of(op->affected_info->id);
        if (index == invalid_mixin_index) return true; // mixin which was removed by rules
        op->mixin_index = index; // assign index
        return false;
    });

    // sort by index
    std::sort(obj_mutates, obj_mutates_end, [](const object_mutate_op* a, const object_mutate_op* b) {
        return a->mixin_index < b->mixin_index;
    });

    // when overriding ops are sorted, we can do a piecewise mutation
    object_mutation obj_mut(obj, type);
    for (auto i = obj_mutates; i != obj_mutates_end; ++i) {
        auto& op = **i;
        obj_mut.default_construct_each_new_mixin(op.mixin_index);
        obj_mut.update_next_mixin([&op](init_new_args args) {
            op.do_init(args);
        }, util::noop_udpate_common_func);
    }
}
}
