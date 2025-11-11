// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <unordered_map>

namespace dynamix {
template <typename Msg> struct msg_traits;
}

#define DYNAMIX_BASIC_DECLARE_DISPATCH(export, dispatch_name, ...) \
    static export dispatch_name { \
        using signature = __VA_ARGS__; \
        using traits = ::dynamix::msg_traits<dispatch_name>; \
        using dispatch_t = std::tuple_element_t<1, traits::args_t>; \
        using map_type = std::unordered_map<dispatch_t, dynamix::feature_info>; \
        static signature& call; \
    }