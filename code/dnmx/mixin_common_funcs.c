// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "mixin_common_funcs.h"
#include "mixin_info.h"

#include <string.h>

dnmx_error_return_t dnmx_mixin_common_init_func(const dnmx_mixin_info* info, void* ptr) {
    memset(ptr, 0, info->size);
    return dnmx_result_success;
}

dnmx_error_return_t dnmx_mixin_common_copy_func(const dnmx_mixin_info* info, void* ptr, const void* src) {
    memcpy(ptr, src, info->size);
    return dnmx_result_success;
}

void dnmx_mixin_common_move_func(const dnmx_mixin_info* info, void* ptr, void* src) {
    memcpy(ptr, src, info->size);
    memset(src, 0, info->size);
}

void dnmx_mixin_common_destroy_func(const dnmx_mixin_info* info, void* ptr) {
    memset(ptr, 0xfe, info->size);
}

int dnmx_mixin_common_cmp_func(const dnmx_mixin_info* info, const void* a, const void* b) {
    return memcmp(a, b, info->size);
}

bool dnmx_mixin_common_eq_func(const dnmx_mixin_info* info, const void* a, const void* b) {
    return memcmp(a, b, info->size) == 0;
}
