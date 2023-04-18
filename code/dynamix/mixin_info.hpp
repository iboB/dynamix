// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/mixin_info.h"
#include "mixin_info_fwd.hpp"
#include "mixin_id.hpp"
#include "error_return.hpp"

namespace dynamix {

struct canonical_mixin_order {
    bool operator()(const mixin_info* a, const mixin_info* b) const noexcept {
        return dnmx_mixin_canonical_order(a, b);
    }
    bool operator()(const mixin_info& a, const mixin_info& b) const noexcept {
        return operator()(&a, &b);
    }
};

} // namespace dynamix
