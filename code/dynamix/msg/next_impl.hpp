// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "msg_traits.hpp"
#include "../object.hpp"
#include "../type.hpp"
#include "../globals.hpp"
#include "../object_of.hpp"
#include "../exception.hpp"

namespace dynamix {
template <typename Msg, typename Mixin>
bool has_next_impl_msg(Mixin* mixin) {
    auto* obj = object_of(mixin);
    return !!obj->get_type().find_next_implementer(Msg::info, g::get_mixin_info<Mixin>());
}

template <typename Msg, typename Mixin, typename... Args>
auto call_next_impl_msg(Mixin* mixin, Args&&... args) -> typename msg_traits<Msg>::ret_t {
    using traits = msg_traits<Msg>;
    auto* obj = static_cast<typename traits::obj_t*>(object_of(mixin));

    auto next = obj->get_type().find_next_implementer(Msg::info, g::get_mixin_info<std::remove_cv_t<Mixin>>());
    if (!next) throw bad_feature_access("next impl");

    return traits::caller::call(*next, *obj, std::forward<Args>(args)...);
}

template <typename Msg, typename Mixin>
bool has_next_bidder_set(Mixin* mixin) {
    auto* obj = object_of(mixin);
    return !obj->get_type().find_next_bidder_set(Msg::info, g::get_mixin_info<Mixin>()).empty();
}

template <typename Msg, typename Mixin, typename... Args>
auto call_next_bidder_set(Mixin* mixin, Args&&... args) -> typename msg_traits<Msg>::ret_t {
    using traits = msg_traits<Msg>;
    auto* obj = static_cast<typename traits::obj_t*>(object_of(mixin));

    auto next = obj->get_type().find_next_bidder_set(Msg::info, g::get_mixin_info<std::remove_cv_t<Mixin>>());
    if (next.empty()) throw bad_feature_access("next bidder set");

    auto begin = next.begin();
    auto back = next.end() - 1;

    for (auto i = back; i != begin; --i) {
        traits::caller::call(*i, *obj, args...); // args are copied! return value is ignored
    }
    // return first (top) result
    return traits::caller::call(*begin, *obj, std::forward<Args>(args)...);
}

}

#define DYNAMIX_HAS_NEXT_IMPL_MSG(msg) ::dynamix::has_next_impl_msg<msg>(this)
#define DYNAMIX_CALL_NEXT_IMPL_MSG(msg, ...) ::dynamix::call_next_impl_msg<msg>(this, ##__VA_ARGS__)

#define DYNAMIX_HAS_NEXT_BIDDER_SET(msg) ::dynamix::has_next_bidder_set<msg>(this)
#define DYNAMIX_CALL_NEXT_BIDDER_SET(msg, ...) ::dynamix::call_next_bidder_set<msg>(this, ##__VA_ARGS__)
