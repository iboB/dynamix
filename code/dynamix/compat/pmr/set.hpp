// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"
#include <set>

#if DYNAMIX_HAS_PMR
namespace dynamix::compat::pmr {
template <typename T, typename Cmp = std::less<T>>
using set = std::pmr::set<T, Cmp>;
}
#else
#include "allocator.hpp"
namespace dynamix::compat::pmr {
template <typename T, typename Cmp = std::less<T>>
using set = std::set<T, Cmp, allocator<T>>;
}
#endif
