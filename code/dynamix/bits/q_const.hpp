// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <type_traits>

namespace dynamix {
template <bool Q, typename T>
using q_const = std::conditional_t<Q, const T, T>;
}
