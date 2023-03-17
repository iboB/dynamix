// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_mutate_ops.hpp"
#include "mixin_info.hpp"
#include "type_mutation.hpp"

namespace dynamix {
struct simple_mutate_op {
    object_mutate_op* to_obj_mutate(const object&) { return nullptr; }
};

struct mutate_op_by_info {
public:
    const dynamix::mixin_info& info;
    mutate_op_by_info(const dynamix::mixin_info& info) : info(info) {}
};

struct mutate_op_by_name {
    std::string_view name;
    mutate_op_by_name(std::string_view name) : name(name) {}
};

struct mutate_op_remove_by_info : public mutate_op_by_info, public simple_mutate_op {
    using mutate_op_by_info::mutate_op_by_info;
    void apply_to_type_mutation(type_mutation& mut) { mut.remove(info); }
};

struct mutate_op_remove_by_name : public mutate_op_by_name, public simple_mutate_op {
    using mutate_op_by_name::mutate_op_by_name;
    void apply_to_type_mutation(type_mutation& mut) { mut.remove(name); }
};

struct mutate_op_just_add_by_info : public mutate_op_by_info, public simple_mutate_op {
    using mutate_op_by_info::mutate_op_by_info;
    void apply_to_type_mutation(type_mutation& mut) { mut.add(info); }
};

struct mutate_op_just_add_by_name : public mutate_op_by_name, public simple_mutate_op {
    using mutate_op_by_name::mutate_op_by_name;
    void apply_to_type_mutation(type_mutation& mut) { mut.add(name); }
};

struct mutate_op_to_back_by_info : public mutate_op_by_info, public simple_mutate_op {
    using mutate_op_by_info::mutate_op_by_info;
    void apply_to_type_mutation(type_mutation& mut) { mut.to_back(info); }
};

struct mutate_op_to_back_by_name : public mutate_op_by_name, public simple_mutate_op {
    using mutate_op_by_name::mutate_op_by_name;
    void apply_to_type_mutation(type_mutation& mut) { mut.to_back(name); }
};

template <typename ConstructNew_Func>
struct add_mixin_op_with_func_by_name final : public mutate_op_by_name, public object_mutate_op_with_func<ConstructNew_Func> {
    add_mixin_op_with_func_by_name(std::string_view name, ConstructNew_Func&& func)
        : mutate_op_by_name(name)
        , object_mutate_op_with_func<ConstructNew_Func>(std::forward<ConstructNew_Func>(func))
    {}
    void apply_to_type_mutation(type_mutation& mut) { this->info_to_add = &mut.add(name); }
};

template <typename ConstructNew_Func>
struct add_mixin_op_with_func_by_info final : public mutate_op_just_add_by_info, public object_mutate_op_with_func<ConstructNew_Func> {
    add_mixin_op_with_func_by_info(const mixin_info& info, ConstructNew_Func&& func)
        : mutate_op_just_add_by_info(info)
        , object_mutate_op_with_func<ConstructNew_Func>(std::forward<ConstructNew_Func>(func))
    {
        this->affected_info = &info;
    }
    using object_mutate_op::to_obj_mutate;
};

template <typename Mixin, typename... Args>
struct add_mixin_op_with_args final : mutate_op_just_add_by_info, public object_mutate_op_with_args<Mixin, Args...> {
    add_mixin_op_with_args(Args&&... args)
        : mutate_op_just_add_by_info(g::get_mixin_info<Mixin>())
        , object_mutate_op_with_args<Mixin, Args...>(std::forward<Args>(args)...)
    {}
    using object_mutate_op::to_obj_mutate;
};

////////////////////////////////////////////
// functions

namespace mutate_ops {
mutate_op_remove_by_info remove(const dynamix::mixin_info& info) { return mutate_op_remove_by_info(info); }
mutate_op_remove_by_name remove(std::string_view name) { return mutate_op_remove_by_name(name); }
template <typename Mixin>
mutate_op_remove_by_info remove() { return mutate_op_remove_by_info(g::get_mixin_info<Mixin>()); }

mutate_op_just_add_by_info add(const dynamix::mixin_info& info) { return mutate_op_just_add_by_info(info); }
mutate_op_just_add_by_name add(std::string_view name) { return mutate_op_just_add_by_name(name); }
template <typename Mixin>
mutate_op_just_add_by_info add() { return mutate_op_just_add_by_info(g::get_mixin_info<Mixin>()); }

template <typename ConstructNew_Func>
add_mixin_op_with_func_by_info<ConstructNew_Func> add(const dynamix::mixin_info& info, ConstructNew_Func&& func) {
    return add_mixin_op_with_func_by_info<ConstructNew_Func>(info, std::forward<ConstructNew_Func>(func));
}
template <typename ConstructNew_Func>
add_mixin_op_with_func_by_name<ConstructNew_Func> add(std::string_view name, ConstructNew_Func&& func) {
    return add_mixin_op_with_func_by_name<ConstructNew_Func>(name, std::forward<ConstructNew_Func>(func));
}

template <typename Mixin, typename... Args>
add_mixin_op_with_args<Mixin, Args...> add(Args&&... args) { return add_mixin_op_with_args<Mixin, Args...>(std::forward<Args>(args)...); }

mutate_op_to_back_by_info to_back(const dynamix::mixin_info& info) { return mutate_op_to_back_by_info(info); }
mutate_op_to_back_by_name to_back(std::string_view name) { return mutate_op_to_back_by_name(name); }
template <typename Mixin>
mutate_op_to_back_by_info to_back() { return mutate_op_to_back_by_info(g::get_mixin_info<Mixin>()); }
}

using mutate_ops::add;
using mutate_ops::remove;
using mutate_ops::to_back;

}
