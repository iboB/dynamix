// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "bits/noexcept.h"
#include "error_return.h"

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

// add noexcept to functions even though they are extern "C" so they can be reused in C++
// this way even C++ configs which allow extern "C" functions to throw will work

typedef struct dnmx_mixin_info dnmx_mixin_info;

// memset to 0
DYNAMIX_API dnmx_error_return_t dnmx_mixin_common_init_func(const dnmx_mixin_info* info, void* ptr) DNMX_NOEXCEPT;

// memcpy
DYNAMIX_API dnmx_error_return_t dnmx_mixin_common_copy_func(const dnmx_mixin_info* info, void* ptr, const void* src) DNMX_NOEXCEPT;

// memcpy and memset source to 0
DYNAMIX_API void dnmx_mixin_common_move_func(const dnmx_mixin_info* info, void* ptr, void* src) DNMX_NOEXCEPT;

// memcpy and don't touch source
static inline void dnmx_mixin_common_move_func_copy(const dnmx_mixin_info* info, void* ptr, void* src) DNMX_NOEXCEPT {
    dnmx_mixin_common_copy_func(info, ptr, src);
}

// no common destroy func; nothing smart to do anyway

// memcmp
// WARNING: memcmp may ruin your day with padding bytes in a struct
// it's generally safe if you always memset to zero (like with dnmx_mixin_common_init_func)
// or if the struct has no padding
// but just in case none of these is added in DNMX_COMMON_INIT_MI
DYNAMIX_API int dnmx_mixin_common_cmp_func(const dnmx_mixin_info* info, const void* a, const void* b) DNMX_NOEXCEPT;
DYNAMIX_API bool dnmx_mixin_common_eq_func(const dnmx_mixin_info* info, const void* a, const void* b) DNMX_NOEXCEPT;

////////////////////////////////////////////////////////////////////

// noop funcs: do nothing (but they do it really fast)
// use carefully
// NOTE: it's somewhat of a dillema whether they should be left inline
// for now leave them as such and allow the linker to squash them if possible

static inline dnmx_error_return_t dnmx_mixin_noop_init_func(const dnmx_mixin_info* i, void* p) DNMX_NOEXCEPT {
    (void)i; (void)p;
    return dnmx_result_success;
}
static inline dnmx_error_return_t dnmx_mixin_noop_copy_func(const dnmx_mixin_info* i, void* p, const void* s) DNMX_NOEXCEPT {
    (void)i; (void)p; (void)s;
    return dnmx_result_success;
}
static inline void dnmx_mixin_noop_move_func(const dnmx_mixin_info* i, void* p, void* s) DNMX_NOEXCEPT {
    (void)i; (void)p; (void)s;
}

// this returns 0
static inline int dnmx_mixin_noop_cmp_func(const dnmx_mixin_info* i, const void* a, const void* b) DNMX_NOEXCEPT {
    (void)i; (void)a; (void)b;
    return 0;
}

#if defined(__cplusplus)
}
#endif