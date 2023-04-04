// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_mutate_ops.hpp"
#include "mixin_info.hpp"
#include "object.hpp"
#include "domain.hpp"

#include <string_view>

namespace dynamix {

template <typename ConstructNew_Func>
struct construct_mixin_op_with_func_by_name final : public object_mutate_op_with_func<ConstructNew_Func> {
    std::string_view name;
    construct_mixin_op_with_func_by_name(std::string_view name, ConstructNew_Func&& func)
        : object_mutate_op_with_func<ConstructNew_Func>(std::forward<ConstructNew_Func>(func))
        , name(name)
    {}
    object_mutate_op* to_obj_mutate(object& obj) {
        this->affected_info = obj.get_domain().get_mixin_info(name);
        return this;
    }
};

namespace mutate_to_ops {
template <typename ConstructNew_Func>
object_mutate_op_with_func<ConstructNew_Func> construct(const dynamix::mixin_info& info, ConstructNew_Func&& func) {
    return object_mutate_op_with_func<ConstructNew_Func>(info, std::forward<ConstructNew_Func>(func));
}
template <typename ConstructNew_Func>
construct_mixin_op_with_func_by_name<ConstructNew_Func> construct(std::string_view name, ConstructNew_Func&& func) {
    return construct_mixin_op_with_func_by_name<ConstructNew_Func>(name, std::forward<ConstructNew_Func>(func));
}

template <typename Mixin, typename... Args>
object_mutate_op_with_args<Mixin, Args...> construct(Args&&... args) { return object_mutate_op_with_args<Mixin, Args...>(std::forward<Args>(args)...); }
}

using mutate_to_ops::construct;
}
