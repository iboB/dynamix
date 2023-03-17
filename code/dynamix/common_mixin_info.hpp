// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mixin_info.hpp"
#include <string_view>

namespace dynamix {

// init with ctor
// prevent copy
// allow construction with name
class common_mixin_info : public mixin_info {
public:
    common_mixin_info()
        : mixin_info(dnmx_make_mixin_info())
    {}

    common_mixin_info(dnmx_sv n)
        : common_mixin_info()
    {
        name = n;
    }

    common_mixin_info(std::string_view n)
        : common_mixin_info(dnmx_sv::from_std(n))
    {}

    common_mixin_info(const common_mixin_info&) = delete;
    common_mixin_info& operator=(const common_mixin_info&) = delete;
};

}
