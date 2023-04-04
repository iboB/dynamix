// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/sv.h"
#include "bits/noexcept.h"
#include "type_handle.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef bool (*dnmx_type_class_match_func)(dnmx_type_handle) DNMX_NOEXCEPT;

typedef struct dnmx_type_class {
    dnmx_sv name;
    dnmx_type_class_match_func matches;
} dnmx_type_class;

#if defined(__cplusplus)
}
#endif
