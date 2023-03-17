// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/mixin_id.h"

namespace dynamix
{
using mixin_id = dnmx_mixin_id;
inline constexpr mixin_id invalid_mixin_id = mixin_id{dnmx_invalid_id};
}
