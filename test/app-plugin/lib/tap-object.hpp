// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "tap-api.h"
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>

namespace tap {
class TAP_API object : public dynamix::object {
public:
    object() noexcept;

    static object* of(void* mixin) {
        return static_cast<object*>(dynamix::object_of(mixin));
    }
    static const object* of(const void* mixin) {
        return static_cast<const object*>(dynamix::object_of(mixin));
    }
};

#define tap_self ::tap::object::of(this)

}
