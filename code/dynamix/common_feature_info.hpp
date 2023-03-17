// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_info.hpp"
#include <string_view>

namespace dynamix {

// init with ctor
// prevent copy
// allow construction with name
class common_feature_info : public feature_info {
public:
    common_feature_info()
        : feature_info(dnmx_make_feature_info())
    {}

    common_feature_info(dnmx_sv n, bool clash = false, dnmx_feature_payload default_pl = nullptr)
        : common_feature_info()
    {
        name = n;
        allow_clashes = clash;
        default_payload = default_pl;
    }

    common_feature_info(const common_feature_info&) = delete;
    common_feature_info& operator=(const common_feature_info&) = delete;
};

}
