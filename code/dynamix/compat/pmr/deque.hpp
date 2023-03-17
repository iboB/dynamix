// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"
#include <deque>

#if DYNAMIX_HAS_PMR
namespace dynamix::compat::pmr {
template <typename T>
using deque = std::pmr::deque<T>;
}
#else
#include "allocator.hpp"
namespace dynamix::compat::pmr {
template <typename T>
using deque = std::deque<T, allocator<T>>;
}
#endif
