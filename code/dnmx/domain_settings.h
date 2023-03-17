// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdbool.h>

// settings for domain creation
// the default values are false for everything
typedef struct dnmx_domain_settings {
    // if true it will automatically sort mixins in an type to a canonical order
    // this means that mixins will be ordered not by the order of creation, but
    // using mixin_info::canonical_order
    // thus a type composed of mixins A,B will be identical to a type composed of
    // first adding B and then A (A-B = B-A)
    // note the the overriding features in this case will have to depend on
    // the feature's bid and priority
    // also note that ordering can be achieved externally
    bool canonicalize_types;

    // if true, enforces that newly registered feature types have unique names
    // will produce an error if a feature with a duplicate name is registered
    // WARNING: if duplicate names exist,
    // any ops involing with feature names will produce unpredictable results
    bool allow_duplicate_feature_names;

    // if true, enforces that newly registered mixin types have unique names
    // will produce an error if a mixin with a duplicate name is registered
    // WARNING: if duplicate names exist,
    // any ops involing with mixin names will produce unpredictable results
    bool allow_duplicate_mixin_names;
} dnmx_domain_settings;
