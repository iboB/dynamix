// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"

#include "domain_handle.h"
#include "basic_domain.h"

#include "feature_id.h"
#include "mixin_id.h"

#include "type_handle.h"

#include "error_return.h"


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_info dnmx_feature_info;
typedef struct dnmx_mixin_info dnmx_mixin_info;

DYNAMIX_API dnmx_domain_handle dnmx_create_domain(dnmx_sv name, dnmx_domain_settings settings, uintptr_t user_data, void* context);
DYNAMIX_API void dnmx_destroy_domain(dnmx_domain_handle hd);

static inline dnmx_sv dnmx_get_domain_name(dnmx_domain_handle hd) {
    return hd->m_name;
}
static inline dnmx_domain_settings dnmx_get_domain_settings(dnmx_domain_handle hd) {
    return hd->m_settings;
}
static inline uintptr_t dnmx_get_domain_user_data(dnmx_domain_handle hd) {
    return hd->user_data;
}
static inline void* dnmx_get_domain_context(dnmx_domain_handle hd) {
    return hd->context;
}
DYNAMIX_API void dnmx_set_domain_user_data(dnmx_domain_handle hd, uintptr_t user_data);
DYNAMIX_API void dnmx_set_domain_context(dnmx_domain_handle hd, void* context);

DYNAMIX_API dnmx_error_return_t dnmx_register_feature(dnmx_domain_handle hd, dnmx_feature_info* info);
DYNAMIX_API void dnmx_unregister_feature(dnmx_domain_handle hd, dnmx_feature_info* info);
DYNAMIX_API dnmx_error_return_t dnmx_register_mixin(dnmx_domain_handle hd, dnmx_mixin_info* info);
DYNAMIX_API void dnmx_unregister_mixin(dnmx_domain_handle hd, dnmx_mixin_info* info);

DYNAMIX_API const dnmx_feature_info* dnmx_get_feature_info_by_id(dnmx_domain_handle hd, dnmx_feature_id id);
DYNAMIX_API const dnmx_feature_info* dnmx_get_feature_info_by_name(dnmx_domain_handle hd, dnmx_sv name);
DYNAMIX_API const dnmx_mixin_info* dnmx_get_mixin_info_by_id(dnmx_domain_handle hd, dnmx_mixin_id id);
DYNAMIX_API const dnmx_mixin_info* dnmx_get_mixin_info_by_name(dnmx_domain_handle hd, dnmx_sv name);

DYNAMIX_API dnmx_type_handle dnmx_get_type(dnmx_domain_handle hd, const dnmx_mixin_info* const* mixins, uint32_t num_mixins);
DYNAMIX_API dnmx_type_handle dnmx_get_empty_type(dnmx_domain_handle hd);
DYNAMIX_API size_t dnmx_get_num_types(dnmx_domain_handle hd);
DYNAMIX_API void dnmx_garbage_collect_types(dnmx_domain_handle hd);

#if defined(__cplusplus)
}
#endif

