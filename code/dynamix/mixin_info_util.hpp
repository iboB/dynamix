// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mixin_info.hpp"
#include "error_return.hpp"

#include "../dnmx/mixin_common_funcs.h"
#include "../dnmx/bits/pp.h"

#include <type_traits>

namespace dynamix::util {

template <typename Mixin>
dnmx_error_return_t call_mixin_default_ctor(const mixin_info*, void* memory) {
    new (memory) Mixin;
    return result_success;
}
template <typename Mixin>
void call_mixin_dtor(const mixin_info*, void* memory) noexcept {
    reinterpret_cast<Mixin*>(memory)->~Mixin();
}
template <typename Mixin>
dnmx_error_return_t call_mixin_copy_ctor(const mixin_info*, void* memory, const void* source) {
    new (memory) Mixin(*reinterpret_cast<const Mixin*>(source));
    return result_success;
}
template <typename Mixin>
dnmx_error_return_t call_mixin_copy_assignment(const mixin_info*, void* target, const void* source) {
    *reinterpret_cast<Mixin*>(target) = *reinterpret_cast<const Mixin*>(source);
    return result_success;
}
template <typename Mixin>
void call_mixin_move_ctor(const mixin_info*, void* memory, void* source) noexcept {
    new (memory) Mixin(std::move(*reinterpret_cast<Mixin*>(source)));
}
template <typename Mixin>
void call_mixin_move_assignment(const mixin_info*, void* target, void* source) noexcept {
    *reinterpret_cast<Mixin*>(target) = std::move(*reinterpret_cast<Mixin*>(source));
}

template <typename Mixin>
bool call_mixin_equality_operator(const mixin_info*, const void* a, const void* b) noexcept {
    static_assert(noexcept(std::declval<Mixin>() == std::declval<Mixin>()));
    return *reinterpret_cast<const Mixin*>(a) == *reinterpret_cast<const Mixin*>(b);
}

// set default values for fields for a given type
// sets type-specific values:
// * size, alignment, obj_buf_size, obj_buf_alignment_and_mixin_offset
// * init, copy_init, move_init, copy_asgn, move_asgn, destroy
// works with void in which case it initializes with noop
// DOES NOT SET:
//  * name
//  * on non-void: equals or compare even if possible
template <typename Mixin>
void common_init_mixin_info(mixin_info& info) {
    if constexpr(std::is_same_v<Mixin, void>) {
        info.init = &dnmx_mixin_noop_init_func;
        info.copy_init = &dnmx_mixin_noop_copy_func;
        info.copy_asgn = &dnmx_mixin_noop_copy_func;
        info.move_init = &dnmx_mixin_noop_move_func;
        info.move_asgn = &dnmx_mixin_noop_move_func;
        info.compare = &dnmx_mixin_noop_cmp_func;
    }
    else {
        info.set_size_alignment(sizeof(Mixin), alignof(Mixin));

        if constexpr (std::is_trivially_constructible_v<Mixin>) info.init = &dnmx_mixin_common_init_func;
        else if constexpr (std::is_default_constructible_v<Mixin>) info.init = &call_mixin_default_ctor<Mixin>;

        if constexpr (std::is_trivially_copy_constructible_v<Mixin>) info.copy_init = &dnmx_mixin_common_copy_func;
        else if constexpr (std::is_copy_constructible_v<Mixin>) info.copy_init = &call_mixin_copy_ctor<Mixin>;

        if constexpr (std::is_trivially_copy_assignable_v<Mixin>) info.copy_asgn = &dnmx_mixin_common_copy_func;
        else if constexpr (std::is_copy_assignable_v<Mixin>) info.copy_asgn = &call_mixin_copy_assignment<Mixin>;

        if constexpr (std::is_trivially_move_constructible_v<Mixin>) info.move_init = &dnmx_mixin_common_move_func;
        else if constexpr (std::is_nothrow_move_constructible_v<Mixin>) info.move_init = &call_mixin_move_ctor<Mixin>;

        if constexpr (std::is_trivially_move_assignable_v<Mixin>) info.move_asgn = &dnmx_mixin_common_move_func;
        else if constexpr (std::is_nothrow_move_assignable_v<Mixin>) info.move_asgn = &call_mixin_move_assignment<Mixin>;

        // no checks for nothrow_destructible
        // explicitly marked noexcept dtors are a rarity, so we just deal with it
        if constexpr (!std::is_trivially_destructible_v<Mixin>) info.destroy = &call_mixin_dtor<Mixin>;
    }
}

#define DYNAMIX_COMMON_INIT_MI(mi, mixin_type) do { \
    mi.name = dnmx_make_sv_lit(I_DNMX_PP_STRINGIZE(mixin_type)); \
    ::dynamix::util::common_init_mixin_info<mixin_type>(mi); \
} while (false)

}
