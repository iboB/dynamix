// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/empty_init.h"
#include "mixin_index.h"
#include "feature_payload.h"
#include "feature_id.h"

#include <splat/inline.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_feature_for_mixin dnmx_feature_for_mixin;

// feature data for the ftable which consists of tighly packed elements for faster acciess
typedef struct dnmx_ftable_payload {
    dnmx_mixin_index_t mixin_index; // index of mixin which provided this payload (never invalid)
    dnmx_feature_payload payload; // copied from feature_for_mixin (to save the indirection)
    const dnmx_feature_for_mixin* data; // associated feature_for_mixin. never null

#if defined(__cplusplus)
    explicit operator bool() const noexcept { return !!payload; }
#endif
} dnmx_ftable_payload;

typedef struct dnmx_ftable_entry {
    // all reachable payloads of this feature:
    // if the feature is not provided, begin = top_bid_back = end = nullptr

    // top-bid, top-prio payload
    // in a clash scenario this is the payload from the last mixin in the type
    // to provide this feature
    const dnmx_ftable_payload* begin;

    // last payload with the top bid
    // disregarding priorities, this is the first mixin by mixin order
    // it may be useful in case one wants to iterate in order of mixins
    const dnmx_ftable_payload* top_bid_back;

    // end of range
    const dnmx_ftable_payload* end;

#if defined(__cplusplus)
    // utils
    explicit operator bool() const noexcept { return !!begin; }
    // itlib::span<const ftable_payload> top_prio_only() const noexcept { return itlib::span(begin, top_bid_back + 1); }
    uint32_t size() const noexcept { return uint32_t(end - begin); }
#endif
} dnmx_ftable_entry;

struct dnmx_basic_type {
    // ftable visible to C so that feature queries can be inlined there too

    // sparse array of ftable entry per feature id
    // an empty entry here may still be "implemented" by this type if it has a default payload
    const dnmx_ftable_entry* ftable;
    uint32_t ftable_length;

#if defined(__cplusplus)
    [[nodiscard]] FORCE_INLINE dnmx_ftable_entry ftable_at(dnmx_feature_id id) const noexcept {
        if (id.i < ftable_length) return ftable[id.i];
        return {nullptr, nullptr, nullptr};
    }
#endif
};

static FORCE_INLINE dnmx_ftable_entry dnmx_ftable_at(const struct dnmx_basic_type* type, dnmx_feature_id id) {
    if (id.i < type->ftable_length) return type->ftable[id.i];
    return DNMX_EMPTY_T(dnmx_ftable_entry);
}

#if defined(__cplusplus)
}
#endif