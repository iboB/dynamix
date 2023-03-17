// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#include "domain.hpp"
#include "mixin_info.hpp"

namespace dynamix {
inline const domain* domain_from_info(const mixin_info& info) {
    return domain::from_c_handle(info.dom);
}
}
