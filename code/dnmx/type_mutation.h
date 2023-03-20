// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "type_mutation_handle.h"

#include "error_return.h"
#include "domain_handle.h"
#include "type_handle.h"
#include "mixin_index.h"

#include "bits/sv.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_info dnmx_feature_info;
typedef struct dnmx_mixin_info dnmx_mixin_info;

DYNAMIX_API dnmx_type_mutation_handle dnmx_create_type_mutation_empty(dnmx_domain_handle hd);
DYNAMIX_API dnmx_type_mutation_handle dnmx_create_type_mutation_from_type(dnmx_type_handle ht);
DYNAMIX_API void dnmx_destroy_unused_type_mutation(dnmx_type_mutation_handle hmut);

DYNAMIX_API dnmx_domain_handle dnmx_type_mutation_get_domain(dnmx_type_mutation_handle hmut);
DYNAMIX_API dnmx_type_handle dnmx_type_mutation_get_base(dnmx_type_mutation_handle hmut);
DYNAMIX_API dnmx_type_template_handle dnmx_type_mutation_get_new_type(dnmx_type_mutation_handle hmut);

DYNAMIX_API void dnmx_type_mutation_reset(dnmx_type_mutation_handle hmut);
DYNAMIX_API bool dnmx_type_mutation_is_noop(dnmx_type_mutation_handle hmut);
DYNAMIX_API bool dnmx_type_mutation_is_adding(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info);
DYNAMIX_API const dnmx_mixin_info* dnmx_type_mutation_is_adding_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name);
DYNAMIX_API bool dnmx_type_mutation_is_adding_mixins(dnmx_type_mutation_handle hmut);
DYNAMIX_API bool dnmx_type_mutation_is_removing(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info);
DYNAMIX_API const dnmx_mixin_info* dnmx_type_mutation_is_removing_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name);
DYNAMIX_API bool dnmx_type_mutation_is_removing_mixins(dnmx_type_mutation_handle hmut);

// type template functions

DYNAMIX_API bool dnmx_type_template_has(dnmx_type_template_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API const dnmx_mixin_info* dnmx_type_template_has_by_name(dnmx_type_template_handle ht, dnmx_sv name);
DYNAMIX_API bool dnmx_type_template_implements_strong(dnmx_type_template_handle ht, const dnmx_feature_info* info);
DYNAMIX_API const dnmx_feature_info* dnmx_type_template_implements_strong_by_name(dnmx_type_template_handle ht, dnmx_sv name);
DYNAMIX_API bool dnmx_type_template_implements(dnmx_type_template_handle ht, const dnmx_feature_info* info);

DYNAMIX_API bool dnmx_type_template_add(dnmx_type_template_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API bool dnmx_type_template_add_if_lacking(dnmx_type_template_handle ht, const dnmx_mixin_info* info);

DYNAMIX_API bool dnmx_type_template_remove(dnmx_type_template_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API const dnmx_mixin_info* dnmx_type_template_remove_by_name(dnmx_type_template_handle ht, dnmx_sv name);
DYNAMIX_API bool dnmx_type_template_to_back(dnmx_type_template_handle ht, const dnmx_mixin_info* info);
DYNAMIX_API const dnmx_mixin_info* dnmx_type_template_to_back_by_name(dnmx_type_template_handle ht, dnmx_sv name);
DYNAMIX_API void dnmx_type_template_dedup(dnmx_type_template_handle ht);

DYNAMIX_API const dnmx_mixin_info* const* dnmx_type_template_get_mixins(dnmx_type_template_handle ht, dnmx_mixin_index_t* out_num_mixins);
DYNAMIX_API bool dnmx_type_template_set_mixins(dnmx_type_template_handle ht, const dnmx_mixin_info* const* mixins, dnmx_mixin_index_t num_mixins);

#if defined(__cplusplus)
}
#endif
