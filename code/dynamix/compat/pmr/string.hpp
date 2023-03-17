// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"
#include <string>

#if DYNAMIX_HAS_PMR
namespace dynamix::compat::pmr {
using string = std::pmr::string;
}
#else
#include "allocator.hpp"
namespace dynamix::compat::pmr {
using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;
}
#endif
