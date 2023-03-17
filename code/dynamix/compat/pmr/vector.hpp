// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"
#include <vector>

#if DYNAMIX_HAS_PMR
namespace dynamix::compat::pmr {
template <typename T>
using vector = std::pmr::vector<T>;
}
#else
#include "allocator.hpp"
namespace dynamix::compat::pmr {
template <typename T>
using vector = std::vector<T, allocator<T>>;
}
#endif
