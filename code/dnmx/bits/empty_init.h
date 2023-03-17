// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

// c and c++ compatible zero init

#if defined(__cplusplus)
#   define DNMX_EMPTY_VAL {}
#   define DNMX_EMPTY_T(T) {}
#else
#define DNMX_EMPTY_VAL {0}
#define DNMX_EMPTY_T(T) (T){0}
#endif
