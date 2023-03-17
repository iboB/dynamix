// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_id.h"
#include "feature_payload.h"

#include "bits/sv.h"

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

// it is not safe to change the values of a feature info once it's registered
typedef struct dnmx_feature_info {
    // will be set by the domain once registered
    dnmx_feature_id id;

    // if domain_settings::unique_feature_names is true (the default),
    // the name must be unique for the domain
    dnmx_sv name;

    // determines whether it's an error to have concurrency clashes in the ftable
    // it false, it's an error to have two mixins to provide the feature with the same bid and priority
    bool allow_clashes;

    // optional default payload for feature
    // if it exists, it can be available for types which don't have a mixin to provide it
    dnmx_feature_payload default_payload;

    // optional value which can be used to categorize different features of the same class
    // not used by the library yet, reserved for future use
    uint64_t feature_class;

#if defined(__cplusplus)
    dnmx_id_int_t iid() const noexcept { return id.i; }
#endif
} dnmx_feature_info;

static inline dnmx_feature_info dnmx_make_feature_info(void) {
    dnmx_feature_info ret = DNMX_EMPTY_VAL;
    ret.id.i = dnmx_invalid_id;
    return ret;
}

#if defined(__cplusplus)
}
#endif
