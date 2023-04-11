// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "error_return.h"
#include "type_mutation_handle.h"
#include "bits/sv.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef dnmx_error_return_t(*dmmx_mutation_rule_apply_func)(dnmx_type_mutation_handle mutation, uintptr_t user_data);

// it is not safe to change the values of a mutation rule info once it's added
// unlike features and mixins, the same rule can be added to multiple domains
// it also safe (noop-like) to add the same mutation rule to the same domain
typedef struct dnmx_mutation_rule_info {
    // optional name to help with debugging
    dnmx_sv name;

    // the function which applies the rule
    // if it returns an error or throws, the mutation is considered flawed and no type will be created
    dmmx_mutation_rule_apply_func apply;

    // optional user data for rule to be provided to apply function
    uintptr_t user_data;

    // sort order of rule
    // higher values will be applied after lower ones
    // mutation rules with the same order priority will be executed by name (lexicographically smaller first)
    // if the names are also the same, executed in an indeterminate order
    int32_t order_priority;

#if defined(__cplusplus)
    using apply_func = dmmx_mutation_rule_apply_func;
#endif
} dnmx_mutation_rule_info;

#if defined(__cplusplus)
}
#endif
