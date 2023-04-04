// Copyrigho (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "bits/sv.h"

#include "object_handle.h"
#include "basic_object.h"

#include "error_return.h"
#include "domain_handle.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_info dnmx_feature_info;
typedef struct dnmx_mixin_info dnmx_mixin_info;

DYNAMIX_API dnmx_object_handle dnmx_create_object_empty(dnmx_domain_handle hd);
DYNAMIX_API dnmx_object_handle dnmx_create_object_default(dnmx_type_handle ht);
DYNAMIX_API dnmx_object_handle dnmx_create_object_copy(dnmx_const_object_handle ho);
DYNAMIX_API dnmx_object_handle dnmx_create_object_usurp(dnmx_object_handle ho);
DYNAMIX_API void dnmx_destroy_object(dnmx_object_handle ho);

DYNAMIX_API dnmx_error_return_t dnmx_object_copy_from(dnmx_object_handle target, dnmx_const_object_handle source);

DYNAMIX_API dnmx_domain_handle dnmx_object_get_domain(dnmx_const_object_handle ho);
DYNAMIX_API dnmx_type_handle dnmx_object_get_type(dnmx_const_object_handle ho);

DYNAMIX_API dnmx_mixin_index_t dnmx_object_num_mixins(dnmx_const_object_handle ho);

DYNAMIX_API bool dnmx_object_has(dnmx_const_object_handle ho, const dnmx_mixin_info* info);
DYNAMIX_API bool dnmx_object_has_by_name(dnmx_const_object_handle ho, dnmx_sv name);

DYNAMIX_API bool dnmx_object_implements_strong(dnmx_const_object_handle ho, const dnmx_feature_info* info);
DYNAMIX_API bool dnmx_object_implements_strong_by_name(dnmx_const_object_handle ho, dnmx_sv name);

DYNAMIX_API const void* dnmx_object_get(dnmx_const_object_handle ho, const dnmx_mixin_info* info);
DYNAMIX_API const void* dnmx_object_get_by_name(dnmx_const_object_handle ho, dnmx_sv name);
DYNAMIX_API const void* dnmx_object_get_at(dnmx_const_object_handle ho, dnmx_mixin_index_t index);
DYNAMIX_API void* dnmx_object_get_mut(dnmx_object_handle ho, const dnmx_mixin_info* info);
DYNAMIX_API void* dnmx_object_get_mut_by_name(dnmx_object_handle ho, dnmx_sv name);
DYNAMIX_API void* dnmx_object_get_mut_at(dnmx_object_handle ho, dnmx_mixin_index_t index);

DYNAMIX_API bool dnmx_object_clear(dnmx_object_handle ho); // false if sealed
DYNAMIX_API dnmx_error_return_t dnmx_object_reset_type(dnmx_object_handle ho, dnmx_type_handle ht);

DYNAMIX_API bool dnmx_object_is_empty(dnmx_const_object_handle ho);

DYNAMIX_API void dnmx_object_seal(dnmx_object_handle ho);
DYNAMIX_API bool dnmx_object_is_sealed(dnmx_const_object_handle ho);

DYNAMIX_API bool dnmx_object_equals(dnmx_const_object_handle a, dnmx_const_object_handle b);

typedef struct dnmx_int_result {
    int result;
    dnmx_error_return_t error;
} dnmx_int_result;

DYNAMIX_API dnmx_int_result dnmx_object_compare(dnmx_const_object_handle a, dnmx_const_object_handle b);

#if defined(__cplusplus)
}
#endif
