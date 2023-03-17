// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdint.h>

// most type-erased generic payload of features
typedef uint32_t dnmx_id_int_t;
#define dnmx_invalid_id (~((dnmx_id_int_t)0))
