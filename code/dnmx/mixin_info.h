// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/sv.h"
#include "bits/noexcept.h"

#include "size.h"
#include "mixin_id.h"
#include "error_return.h"

#include "mixin_allocator_handle.h"
#include "domain_handle.h"

#include "feature_for_mixin.h"

#if defined(__cplusplus)
#include <itlib/span.hpp>
extern "C" {
#endif

typedef struct dnmx_mixin_info dnmx_mixin_info;

// the arguments to these functions are never null, even if the mixin size is zero
typedef dnmx_error_return_t(*dnmx_mixin_init_func)(const dnmx_mixin_info* info, void* ptr);
typedef dnmx_error_return_t(*dnmx_mixin_copy_func)(const dnmx_mixin_info* info, void* ptr, const void* src);
typedef void(*dnmx_mixin_move_func)(const dnmx_mixin_info* info, void* ptr, void* src) DNMX_NOEXCEPT;
typedef void(*dnmx_mixin_destroy_func)(const dnmx_mixin_info* info, void* ptr) DNMX_NOEXCEPT;
typedef bool(*dnmx_mixin_eq_func)(const dnmx_mixin_info* info, const void* a, const void* b) DNMX_NOEXCEPT;
typedef int(*dnmx_mixin_cmp_func)(const dnmx_mixin_info* info, const void* a, const void* b) DNMX_NOEXCEPT;

// this needs to be filled before registering mixins
// default values can be obtained via dnmx_make_mixin_info()
// after a mixin is registered, changing any of tese values
// is forbidden and will lead to UB - crashes or bugs
// the only exception to the rule above is user_data
struct dnmx_mixin_info {
    // will be set by the domain once registered
    // having a valid id implies the type info is registered with some domain
    // to register a type in multiple domains, you need make copies of the info per domain
    dnmx_mixin_id id;
    dnmx_domain_handle dom;

    // name of mixin
    // if domain_settings::unique_mixin_names is true (the default),
    // the name must be unique for the domain
    // this is the only value without a sane default
    dnmx_sv name;

    // size and alignent of mixin
    // can both be zero
    // if size is not zero, alignment must be a power of 2
    // use set_size_alignment or dnmx_set_mixin_info_size_and_alignment to set
    dnmx_byte_size_t size;
    dnmx_byte_size_t alignment;

    // used if the mixin is external
    // set by set_size_alignment or dnmx_set_mixin_info_size_and_alignment
    dnmx_byte_size_t obj_buf_size;

    // the mixin offset in the external buffer is always one alignment in.
    // this alignment is not necessarily the one of the mixin, though
    // set by set_size_alignment or dnmx_set_mixin_info_size_and_alignment
    dnmx_byte_size_t obj_buf_alignment_and_mixin_offset;

    // any of the following can be null, which implies that this action is invalid and forbidden
    // even if you have nothing to do in such a function, you must set a pointer here to mark
    // the action as valid, otherwise exceptions will be thrown where one of these was
    // supposed to be called mixin_common_funcs.h and mixin_info_util.hpp provide common implementations
    dnmx_mixin_init_func init;
    dnmx_mixin_copy_func copy_init;
    dnmx_mixin_move_func move_init; // leaving nullptr here makes the mixin implicitly external
    dnmx_mixin_copy_func copy_asgn;
    dnmx_mixin_move_func move_asgn;

    // contrary to the functions above, leaving null here implies that there is nothing to be
    // done when the object is destroyed.
    // this means that mixins which are logically destroyed will safely do nothing at this point
    // and no error will be produced
    dnmx_mixin_destroy_func destroy;

    // optional compare functions to be used when comparing objects with such mixins
    // if `equals` is null compare is used on object::equals
    dnmx_mixin_eq_func equals;
    dnmx_mixin_cmp_func compare;

    // optional allocator for this mixin
    // the user is responsible for preserving the lifetime of the allocator while
    // the mixin is registered
    // the presense of an allocator makes the mixin implicitly external
    dnmx_mixin_allocator_handle allocator;

    // optional value which can be used to categorize different mixins in a type
    // not used by the library yet, reserved for future use
    uint64_t mixin_class;

    // optional value which can be used to categorize type infos
    // it may be useful if one reuses functions for different type infos
    // not used by the library yet, reserved for future use
    uint64_t type_info_class;

    // optional user data which is accessible where the info is provided
    // the library does not touch this value
    // it's safe to change even for registered mixins
    // (as safe as the user logic allows it)
    uintptr_t user_data;

    // features provided by this mixin
    // will be used by the domain for new types
    // the domain will also automatically register unregistered features from here
    const dnmx_feature_for_mixin* features;
    uint32_t num_features;

    // key for canonical order
    // if domain_settings::canonical_types is true, this is used to sort mixins in types
    // it is also used to compare types. Types are compare by lexicographical canonical mixin order
    // the smaller the number, the earlier the mixin will appear in an object
    // will also affect multicast order if not otherwise specified
    // mixins with the same order_priority are compared by name
    int32_t order_priority;

    // force the mixin to be external
    // an external mixin is not part of the object buffer, but allocated separately
    // if this is false:
    // * mixins with an allocator are external
    // * mixins without a move constructor are external
    // * all others are internal
    // an external mixin with no allocator is allocated by the object's allocator
    bool force_external;

    // mark the mixin as dependency
    // every mutation will start by removing dependencies from the source type
    // they are supposed to be added by mutation rules alone
    bool dependency;

#if defined(__cplusplus)
    dnmx_id_int_t iid() const noexcept { return id.i; }
    bool registered() const noexcept { return id.i != dnmx_invalid_id; }
    [[nodiscard]] bool external() const noexcept {
        return !move_init || allocator || force_external;
    }
    void set_size_alignment(dnmx_byte_size_t s, dnmx_byte_size_t a) noexcept;

    itlib::span<const dnmx_feature_for_mixin> features_span() const noexcept {
        return {features, num_features};
    }
    void set_features(itlib::span<const dnmx_feature_for_mixin> f) noexcept {
        features = f.data();
        num_features = f.size();
    }
#endif
};

// init with zeroes and set non-zero defaults
static inline dnmx_mixin_info dnmx_make_mixin_info() {
    dnmx_mixin_info ret = DNMX_EMPTY_VAL;
    ret.id.i = dnmx_invalid_id;
    ret.obj_buf_size = dnmx_sizeof_ptr;
    ret.obj_buf_alignment_and_mixin_offset = dnmx_sizeof_ptr;
    return ret;
}

// implementation of canonical order
static inline bool dnmx_mixin_canonical_order(const dnmx_mixin_info* a, const dnmx_mixin_info* b) {
    int32_t ca = a->order_priority, cb = b->order_priority;
    if (ca != cb) return ca < cb;
    int cmp = dnmx_sv_cmp(a->name, b->name);
    if (cmp) return cmp < 0;
    // in case the domain allows duplicate mixin names, we still have to do something here
    // we can no longer guarantee the same order across recompilations, but we can try
    // to make it work across reruns
    if (a->id.i != b->id.i) return a->id.i < b->id.i;
    return a < b; // last resort
}

static inline void dnmx_set_mixin_info_size_and_alignment(dnmx_mixin_info* info, dnmx_byte_size_t s, dnmx_byte_size_t a) {
    info->size = s;
    info->alignment = a;

    if (a < dnmx_sizeof_ptr) {
        // if alignment is less than sizeof_ptr, we would still have to align the object pointer in the buf
        // so we force the alignment to fit the pointer
        info->obj_buf_size = info->size + dnmx_sizeof_ptr;
        info->obj_buf_alignment_and_mixin_offset = dnmx_sizeof_ptr;
    }
    else {
        info->obj_buf_size = info->size + info->alignment;
        info->obj_buf_alignment_and_mixin_offset = info->alignment;
    }
}

#if defined(__cplusplus)
inline void dnmx_mixin_info::set_size_alignment(dnmx_byte_size_t s, dnmx_byte_size_t a) noexcept {
    dnmx_set_mixin_info_size_and_alignment(this, s, a);
}

} // extern "C"
#endif
