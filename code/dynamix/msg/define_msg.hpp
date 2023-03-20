// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "msg_traits.hpp"
#include "msg_macros.hpp"

#define DYNAMIX_DEFINE_SIMPLE_MSG_EX(msg_name, mechanism, clash, default_impl) \
    msg_name::traits::sig_t& msg_name::call = msg_name::traits::I_DNMX_PP_CAT(call_, mechanism); \
    const ::dynamix::common_feature_info& msg_name::get_info_safe() { \
        static ::dynamix::common_feature_info the_info(dnmx_make_sv_lit(I_DNMX_PP_STRINGIZE(msg_name)), clash, (void*)(static_cast<msg_name::traits::sig_t*>(default_impl))); \
        return the_info; \
    } \
    const ::dynamix::common_feature_info& msg_name::info = msg_name::get_info_safe()

#define DYNAMIX_DEFINE_SIMPLE_MSG(msg_name, mechanism) \
    DYNAMIX_DEFINE_SIMPLE_MSG_EX(msg_name, mechanism, true, nullptr)

#define DYNAMIX_DEFINE_MSG_EX(msg_name, mechanism, clash, default_impl, func_name, return_type, args) \
    return_type func_name(I_DNMX_DECL_ARGS args) { \
        return msg_name::traits::caller::I_DNMX_PP_CAT(call_, mechanism)(msg_name::info, I_DNMX_FWD_ARGS args); \
    } \
    DYNAMIX_DEFINE_SIMPLE_MSG_EX(msg_name, mechanism, clash, default_impl)

#define DYNAMIX_DEFINE_MSG(msg_name, mechanism, func_name, return_type, args) \
    DYNAMIX_DEFINE_MSG_EX(msg_name, mechanism, true, nullptr, func_name, return_type, args)
