// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "size.hpp"

namespace dynamix::util {

// rounds s up to the nearest multiple of n
inline constexpr byte_size_t next_multiple(byte_size_t s, byte_size_t n) {
    return ((s + n - 1) / n) // divide rounding up
        * n;  // and scale
}

}
