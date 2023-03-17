// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/error_return.h"

namespace dynamix {
using error_return_t = dnmx_error_return_t;
inline constexpr error_return_t result_success = dnmx_result_success;
}
