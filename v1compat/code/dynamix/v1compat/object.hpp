// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include <dynamix/object.hpp>

namespace dynamix::v1compat {
class DYNAMIX_V1COMPAT_API object : public dynamix::object {
public:
    object() noexcept;
    using dynamix::object::object;

    bool implements(const feature_info& f) const noexcept;

    template <typename Feature>
    bool implements(const Feature*) const noexcept {
        return implements(Feature::info);
    }
};
}
