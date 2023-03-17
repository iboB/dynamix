// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/feature_id.h"

namespace dynamix
{
using feature_id = dnmx_feature_id;
inline constexpr feature_id invalid_feature_id = feature_id{dnmx_invalid_id};
}
