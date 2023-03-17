// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdint.h>

// index of mixin within an object type
typedef uint32_t dnmx_mixin_index_t;
#define dnmx_invalid_mixin_index (~((dnmx_mixin_index_t)0))
