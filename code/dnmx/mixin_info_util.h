// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mixin_info.h"
#include "mixin_common_funcs.h"

#include "bits/pp.h"

#include <stdalign.h>

#if defined(__cplusplus)
#   error "Don't use this file for C++ code. Use dynamix/mixin_info_util.hpp"
#endif

// does not set compare or equals since they're potentially dangerous
#define DNMX_COMMON_INIT_MI(mi, mixin_type) do { \
    (mi) = dnmx_make_mixin_info(); \
    (mi).name = dnmx_make_sv_lit(I_DNMX_PP_STRINGIZE(mixin_type)); \
    dnmx_set_mixin_info_size_and_alignment(&(mi), sizeof(mixin_type), alignof(mixin_type)); \
    (mi).init = dnmx_mixin_common_init_func; \
    (mi).move_init = dnmx_mixin_common_move_func; \
    (mi).move_asgn = dnmx_mixin_common_move_func; \
    (mi).copy_init = dnmx_mixin_common_copy_func; \
    (mi).copy_asgn = dnmx_mixin_common_copy_func; \
} while (false)

#define DNMX_COMMON_INIT_EMPTY_MI(mi, mixin_name) do { \
    (mi) = dnmx_make_mixin_info(); \
    (mi).name = dnmx_make_sv_lit(mixin_name); \
    (mi).init = dnmx_mixin_noop_init_func; \
    (mi).move_init = dnmx_mixin_noop_move_func; \
    (mi).move_asgn = dnmx_mixin_noop_move_func; \
    (mi).copy_init = dnmx_mixin_noop_copy_func; \
    (mi).copy_asgn = dnmx_mixin_noop_copy_func; \
    (mi).compare = dnmx_mixin_noop_cmp_func; \
} while (false)