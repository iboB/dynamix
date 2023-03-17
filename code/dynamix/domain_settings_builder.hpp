// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "domain_settings.hpp"

namespace dynamix {
// C++20 makes this type obsolete
struct domain_settings_builder {
    domain_settings settings = {};
    domain_settings_builder& canonicalize_types(bool val = true) noexcept {
        settings.canonicalize_types = val;
        return *this;
    }
    domain_settings_builder& allow_duplicate_feature_names(bool val = true) noexcept {
        settings.allow_duplicate_feature_names = val;
        return *this;
    }
    domain_settings_builder& allow_duplicate_mixin_names(bool val = true) noexcept {
        settings.allow_duplicate_mixin_names = val;
        return *this;
    }
    // intentionally implicit
    operator domain_settings() const noexcept { return settings; }
};
}
