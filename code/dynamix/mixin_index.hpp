// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/mixin_index.h"

namespace dynamix {
// index of mixin within an object type
using mixin_index_t = dnmx_mixin_index_t;
inline constexpr mixin_index_t invalid_mixin_index = dnmx_invalid_mixin_index;
}
