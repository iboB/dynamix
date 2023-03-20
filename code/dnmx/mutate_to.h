// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "object_handle.h"
#include "type_handle.h"
#include "error_return.h"
#include "mixin_index.h"

typedef struct dnmx_mixin_info dnmx_mixin_info;
typedef dnmx_error_return_t(*dnmx_init_override_func)(const dnmx_mixin_info* info, dnmx_mixin_index_t index, uintptr_t user_data, void* ptr);

typedef struct dnmx_init_override {
    const dnmx_mixin_info* mixin;
    dnmx_init_override_func init;
    uintptr_t user_data; // user data for this override
};

DYNAMIX_API dnmx_error_return_t dnmx_mutate_to(dnmx_object_handle obj, dnmx_type_handle type, const dnmx_init_override* overrides, dnmx_mixin_index_t num_overrides);
