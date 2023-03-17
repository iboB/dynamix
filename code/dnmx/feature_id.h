// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/id_int_t.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_id {
    dnmx_id_int_t i;

#if defined(__cplusplus)
    using int_t = dnmx_id_int_t;
    constexpr bool operator==(dnmx_feature_id o) const noexcept { return i == o.i; }
    constexpr bool operator!=(dnmx_feature_id o) const noexcept { return i != o.i; }
#endif
} dnmx_feature_id;

#if defined(__cplusplus)
}
#endif
