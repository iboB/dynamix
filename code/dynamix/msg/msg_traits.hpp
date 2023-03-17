// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "msg_caller.hpp"
#include "../any.hpp"
#include "../bits/q_const.hpp"

namespace dynamix {
template <typename Msg, typename Func>
struct basic_msg_traits;

template <typename Msg, typename Ret, typename Obj, typename... Args>
struct basic_msg_traits<Msg, Ret(Obj, Args...)> {
    using msg_t = Msg;
    using sig_t = Ret(Obj, Args...);

    static constexpr int arity = sizeof...(Args);
    using ret_t = Ret;
    static_assert(std::is_reference_v<Obj>);
    using obj_t = std::remove_reference_t<Obj>;
    static constexpr bool is_const = std::is_const_v<obj_t>;

    using caller = msg_caller<Obj, Ret, Args...>;
    static Ret call_unicast(Obj obj, Args... args) {
        return caller::call_unicast(Msg::info, obj, std::forward<Args>(args)...);
    }
    static Ret call_multicast(Obj obj, Args... args) {
        return caller::call_multicast(Msg::info, obj, std::forward<Args>(args)...);
    }

    using void_t = q_const<is_const, void*>;
    using func_t = Ret(*)(void_t, Args...);

    static any make_payload_by(allocator, void*, func_t func) {
        return fwd_any(reinterpret_cast<feature_payload>(func));
    }

    template <typename Mixin, typename Func>
    static any make_payload_by(allocator, Mixin*, Func f) {
        // static_assert(is_const <= std::is_const_v<Mixin>, "cast loses const qualifier"); // don't allow const mixin to non-const message
        Ret(*pf)(q_const<is_const, Mixin>*, Args...) = f;
        return fwd_any(reinterpret_cast<feature_payload>(pf));
    }
};

template <typename Msg>
struct msg_traits : public basic_msg_traits<Msg, typename Msg::signature> {};
}
