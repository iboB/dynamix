// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/common_feature_info.hpp>

#define DYNAMIX_V1_DEFINE_MESSAGE(message_name) \
    const ::dynamix::common_feature_info& I_DYNAMIX_V1_MSG_STRUCT_NAME(message_name)::get_info_safe() { \
        static ::dynamix::common_feature_info the_info(dnmx_make_sv_lit(I_DNMX_PP_STRINGIZE(message_name)), multicast, nullptr); \
        return the_info; \
    } \
    const ::dynamix::feature_info& I_DYNAMIX_V1_MSG_STRUCT_NAME(message_name)::info = I_DYNAMIX_V1_MSG_STRUCT_NAME(message_name)::get_info_safe(); \
    I_DYNAMIX_V1_MSG_STRUCT_NAME(message_name) * I_DYNAMIX_V1_MESSAGE_TAG(message_name) = nullptr
