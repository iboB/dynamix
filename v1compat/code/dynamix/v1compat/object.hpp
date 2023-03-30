// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>

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

    static object* of(void* mixin) {
        return static_cast<object*>(dynamix::object_of(mixin));
    }
    static const object* of(const void* mixin) {
        return static_cast<const object*>(dynamix::object_of(mixin));
    }
};

#define dm_v1_this ::dynamix::v1compat::object::of(this)
}
