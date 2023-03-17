// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdint.h>

// no feasible use of the library should involve byte_sizes which overflow uint32_t
// (but use a typedef just in case)
typedef uint32_t dnmx_byte_size_t;
#define dnmx_sizeof_ptr ((dnmx_byte_size_t)sizeof(void*))

typedef uint8_t dnmx_byte_t;
