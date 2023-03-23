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
typedef dnmx_error_return_t(*dnmx_mutate_func)(const dnmx_mixin_info* info, uintptr_t user_data, dnmx_mixin_index_t index, void* mixin);

typedef struct dnmx_mutate_to_override {
    const dnmx_mixin_info* mixin; // can be null in which case mixin_name is checked
    dnmx_sv mixin_name; // will only be checked in mixin is null
    dnmx_mutate_func init_new; // called for new mixins (only in new type)
    dnmx_mutate_func update_common; // called for common mixins (in new_type and old object type)
    uintptr_t user_data; // user data for this override
} dnmx_mutate_to_override;
DYNAMIX_API dnmx_error_return_t dnmx_mutate_to(dnmx_object_handle obj, dnmx_type_handle type, const dnmx_mutate_to_override* overrides, uint32_t num_overrides);

enum dnmx_mutate_op_type {
    dnmx_mutate_op_add,
    dnmx_mutate_op_remove,
    dnmx_mutate_op_to_back,
};

typedef struct dnmx_mutate_op {
    int type; // dnmx_mutate_op_type
    const dnmx_mixin_info* mixin; // can be null, in which case mixin_name is checked
    dnmx_sv mixin_name; // will only be checked in mixin is null
    dnmx_mutate_func init_override; // can be null, in which case the default init is used
    uintptr_t user_data; // user data for this op
} dnmx_mutate_op;
DYNAMIX_API dnmx_error_return_t dnmx_mutate(dnmx_object_handle obj, const dnmx_mutate_op* ops, uint32_t num_mutate_ops);

// shorthand one-off syntax
DYNAMIX_API dnmx_error_return_t dnmx_mutate_add(dnmx_object_handle obj, const dnmx_mixin_info* info, dnmx_mutate_func init_override);
DYNAMIX_API dnmx_error_return_t dnmx_mutate_add_by_name(dnmx_object_handle obj, dnmx_sv name, dnmx_mutate_func init_override);
DYNAMIX_API dnmx_error_return_t dnmx_mutate_remove(dnmx_object_handle obj, const dnmx_mixin_info* info);
DYNAMIX_API dnmx_error_return_t dnmx_mutate_remove_by_name(dnmx_object_handle obj, dnmx_sv name);

#if defined(__cplusplus)
}
#endif
