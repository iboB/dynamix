// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../../dnmx/msg/msg_macros.h"
#include "../../dnmx/bits/pp.h"
#include "msg_macros.hpp"

namespace dynamix {
class common_feature_info;
template <typename Msg> struct msg_traits;
}

#define DYNAMIX_BASIC_DECLARE_MSG(export, msg_name, msg_traits, ...) \
    struct export msg_name { \
        using signature = __VA_ARGS__; \
        using traits = msg_traits<msg_name>; \
        static const ::dynamix::common_feature_info& get_info_safe(); \
        static const ::dynamix::common_feature_info& info; \
        static signature& call; \
    }

#define DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(export, msg_name, ...) \
    DYNAMIX_BASIC_DECLARE_MSG(export, msg_name, ::dynamix::msg_traits, __VA_ARGS__)

#define DYNAMIX_DECLARE_SIMPLE_MSG(msg_name, ...) \
    DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(I_DNMX_PP_EMPTY(), msg_name, __VA_ARGS__)

#define DYNAMIX_DECLARE_EXPORTED_MSG_EX(export, msg_name, msg_traits, func_name, return_type, args) \
    DYNAMIX_BASIC_DECLARE_MSG(export, msg_name, msg_traits, return_type args); \
    export return_type func_name args

#define DYNAMIX_DECLARE_EXPORTED_MSG(export, msg_name, func_name, return_type, args) \
    template <typename Msg> struct DYNAMIX_FUNC_TRAITS_NAME(func_name); \
    DYNAMIX_DECLARE_EXPORTED_MSG_EX(export, msg_name, DYNAMIX_FUNC_TRAITS_NAME(func_name), func_name, return_type, args);

#define DYNAMIX_DECLARE_MSG(msg_name, func_name, return_type, args) \
    DYNAMIX_DECLARE_EXPORTED_MSG(I_DNMX_PP_EMPTY(), msg_name, func_name, return_type, args)
