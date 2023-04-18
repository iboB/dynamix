// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_mixin_data.h"
#include "mixin_index.h"
#include "type_handle.h"

#include <splat/inline.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct dnmx_basic_object {
    // members prefixed with m_ indicate that they are "private"
    // and direct access is not recommended

    // type. never null
    dnmx_type_handle m_type;

    // multi-purpose pointer
    // first, it points to the unified buffer for this object
    // the unified buffer contains the array of object_mixin_data per each mixin
    // followed by memory for individual internal mixins
    // m_mixin_data is also an array with an element for each mixins
    // each element contains two pointers:
    // * buf: pointing to the memory allocated for an external mixin
    //        or nullptr if tnis is an internal mixin
    // * mixin: pointer to the mixin within its external buffer
    //          or within the unified obj buffer
    // (each mixin pointer is prepended by sizeof(void*) bytes which contain
    // a pointer to *this* object
    dnmx_object_mixin_data* m_mixin_data;

#if defined(__cplusplus)
    // no bounds check on these ones!
    FORCE_INLINE void* unchecked_get_at(dnmx_mixin_index_t index) noexcept { return m_mixin_data[index].mixin; }
    FORCE_INLINE const void* unchecked_get_at(dnmx_mixin_index_t index) const noexcept { return m_mixin_data[index].mixin; }
#endif
};

static FORCE_INLINE void* dnmx_object_unchecked_get_mut_at(struct dnmx_basic_object* obj, dnmx_mixin_index_t index) {
    return obj->m_mixin_data[index].mixin;
}
static FORCE_INLINE const void* dnmx_object_unchecked_get_at(const struct dnmx_basic_object* obj, dnmx_mixin_index_t index) {
    return obj->m_mixin_data[index].mixin;
}

#if defined(__cplusplus)
}
#endif
