// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_sv dnmx_sv;

bool c_to_cxx(dnmx_sv* a, dnmx_sv* b);
bool cxx_to_c(dnmx_sv* a, dnmx_sv* b);

#if defined(__cplusplus)
}
#endif

