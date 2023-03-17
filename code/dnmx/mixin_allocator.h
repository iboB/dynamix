// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "mixin_allocator_handle.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_mixin_info dnmx_mixin_info;

typedef void* (*dnmx_alloc_mixin_func)(const dnmx_mixin_info*);
typedef void (*dnmx_dealloc_mixin_func)(void*, const dnmx_mixin_info*);

DYNAMIX_API dnmx_mixin_allocator_handle dnmx_create_mixin_allocator(dnmx_alloc_mixin_func alloc, dnmx_dealloc_mixin_func dealloc);
DYNAMIX_API void dnmx_destroy_mixin_allocator(dnmx_mixin_allocator_handle ha);

#if defined(__cplusplus)
}
#endif
