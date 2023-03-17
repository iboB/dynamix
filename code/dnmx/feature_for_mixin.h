// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_payload.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_info dnmx_feature_info;

// a structure that describes a feature with the payload for a concrete mixin
typedef struct dnmx_feature_for_mixin {
    // pointer to registered feature info
    const dnmx_feature_info* info;

    dnmx_feature_payload payload;

    // perks
    int32_t bid;
    int32_t priority;
} dnmx_feature_for_mixin;

#if defined(__cplusplus)
}
#endif
