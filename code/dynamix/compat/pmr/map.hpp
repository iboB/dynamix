// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIK
//
#pragma once
#include "pmr.hpp"
#include <map>

#if DYNAMIX_HAS_PMR
namespace dynamix::compat::pmr {
template <typename K, typename V, typename Cmp = std::less<K>>
using map = std::pmr::map<K, V, Cmp>;
}
#else
#include "allocator.hpp"
namespace dynamix::compat::pmr {
template <typename K, typename V, typename Cmp = std::less<K>>
using map = std::map<K, V, Cmp, allocator<std::pair<const K, V>>>;
}
#endif
