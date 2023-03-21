// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "msg_traits.hpp"
#include "msg_macros.hpp"

#define DYNAMIX_MAKE_FUNC_TRAITS(func_name) \
    template <typename Msg, typename Func> \
    struct I_DYNAMIX_BASIC_FUNC_TRAITS(func_name); \
    template <typename Msg, typename Ret, typename Obj, typename... Args> \
    struct I_DYNAMIX_BASIC_FUNC_TRAITS(func_name)<Msg, Ret(Obj, Args...)> : public ::dynamix::msg_traits<Msg> { \
        using super = ::dynamix::msg_traits<Msg>; \
        static_assert(std::is_same_v<typename super::sig_t, Ret(Obj, Args...)>, "func_traits sig_t mismatch"); \
        template <typename Mixin> \
        static ::dynamix::any make_payload_for(::dynamix::allocator, Mixin*) { \
            using mixin_t = ::dynamix::q_const<super::is_const, Mixin>;\
            static typename super::func_t func = [](typename super::void_t vm, Args... args) -> Ret { \
                mixin_t* m = static_cast<mixin_t*>(vm); \
                return m->func_name(std::forward<Args>(args)...); \
            }; \
            return ::dynamix::fwd_any(reinterpret_cast<::dynamix::feature_payload>(func)); \
        } \
    }; \
    template <typename Msg> \
    struct DYNAMIX_FUNC_TRAITS_NAME(func_name) : public I_DYNAMIX_BASIC_FUNC_TRAITS(func_name)<Msg, typename Msg::signature> {}
