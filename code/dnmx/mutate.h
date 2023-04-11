// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "object_handle.h"
#include "type_handle.h"
#include "error_return.h"
#include "mixin_index.h"
#include "bits/sv.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_mixin_info dnmx_mixin_info;

typedef struct dnmx_mutate_func_args {
    const dnmx_mixin_info* info; // never null
    void* mixin_buf; // buffer of mixin to be initialized or updated, never null
    dnmx_type_handle target_type; // never null
    dnmx_type_handle source_type; // not null in init_new, null in update_common
    dnmx_mixin_index_t target_index; // index of info in target type
    dnmx_mixin_index_t source_index; // unreliable in source_type is null (not guaranteed to be invalid)
    uintptr_t user_data; // user data from op
} dnmx_mutate_func_args;

typedef dnmx_error_return_t(*dnmx_mutate_func)(dnmx_mutate_func_args);

typedef struct dnmx_mutate_to_override {
    const dnmx_mixin_info* mixin; // can be null in which case mixin_name is checked
    dnmx_sv mixin_name; // will only be checked if mixin is null
    dnmx_mutate_func init_new; // called for new mixins (only in new type)
    dnmx_mutate_func update_common; // called for common mixins (in new_type and old object type)
    uintptr_t user_data; // user data for this override
} dnmx_mutate_to_override;
DYNAMIX_API dnmx_error_return_t dnmx_mutate_to(dnmx_object_handle obj, dnmx_type_handle type, const dnmx_mutate_to_override* overrides, uint32_t num_overrides);

enum dnmx_mutate_op_type {
    dnmx_mutate_op_add, // add mixin
    dnmx_mutate_op_remove, // remove mixin
    dnmx_mutate_op_to_back, // move mixin to back
};
typedef struct dnmx_mutate_op {
    int op_type; // dnmx_mutate_op_type
    const dnmx_mixin_info* mixin; // can be null, in which case mixin_name is checked
    dnmx_sv mixin_name; // will only be checked if mixin is null

    // only used on dnmx_mutate_op_add
    // can be null, in which case the default init is used
    dnmx_mutate_func init_override;

    uintptr_t user_data; // user data for this op
} dnmx_mutate_op;
DYNAMIX_API dnmx_error_return_t dnmx_mutate(dnmx_object_handle obj, const dnmx_mutate_op* ops, uint32_t num_mutate_ops);

#if defined(__cplusplus)
}
#endif
