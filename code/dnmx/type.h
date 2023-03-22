// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "bits/sv.h"

#include "type_handle.h"
#include "basic_type.h"

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_info dnmx_feature_info;
typedef struct dnmx_mixin_info dnmx_mixin_info;

DYNAMIX_API size_t dnmx_type_num_objects(dnmx_type_handle ht);
DYNAMIX_API dnmx_mixin_index_t dnmx_type_num_mixins(dnmx_type_handle ht);

DYNAMIX_API bool dnmx_type_has(dnmx_type_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API bool dnmx_type_has_by_name(dnmx_type_handle ht, dnmx_sv name);

DYNAMIX_API dnmx_mixin_index_t dnmx_type_index_of(dnmx_type_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API dnmx_mixin_index_t dnmx_type_index_of_name(dnmx_type_handle ht, dnmx_sv name);

DYNAMIX_API bool dnmx_type_implements_strong(dnmx_type_handle ht, const dnmx_feature_info* info);
DYNAMIX_API bool dnmx_type_implements_strong_by_name(dnmx_type_handle ht, dnmx_sv name);

DYNAMIX_API bool dnmx_type_implements(dnmx_type_handle ht, const dnmx_feature_info* info);

//DYNAMIX_API dnmx_mixin_index_t dnmx_type_is_of();

DYNAMIX_API bool dnmx_type_is_default_constructible(dnmx_type_handle ht);
DYNAMIX_API bool dnmx_type_is_copy_constructible(dnmx_type_handle ht);
DYNAMIX_API bool dnmx_type_is_copy_assignable(dnmx_type_handle ht);
static inline bool dnmx_type_is_copyable(dnmx_type_handle ht) {
    return dnmx_type_is_copy_constructible(ht)
        && dnmx_type_is_copy_assignable(ht);
}
DYNAMIX_API bool dnmx_type_is_equality_comparable(dnmx_type_handle ht);
DYNAMIX_API bool dnmx_type_is_comparable(dnmx_type_handle ht);

#if defined(__cplusplus)
}
#endif
