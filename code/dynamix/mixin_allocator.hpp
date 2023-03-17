// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "../dnmx/mixin_allocator_handle.h"

#include "size.hpp"
#include "mixin_info_fwd.hpp"

namespace dynamix {

// optional allocator for mixins
struct DYNAMIX_API mixin_allocator {
    virtual ~mixin_allocator() = 0;

    // allocate external mixin buffer for an object
    // note that this has to return at least:
    // size = info.desc.obj_buf_size()
    // alignment = info.desc.obj_buf_alignment_and_mixin_offset()
    [[nodiscard]] virtual byte_t* alloc_mixin_buf(const mixin_info& info) = 0;
    virtual void dealloc_mixin_buf(byte_t* ptr, const mixin_info& info) noexcept = 0;

    dnmx_mixin_allocator_handle to_c_hanlde() noexcept { return reinterpret_cast<dnmx_mixin_allocator_handle>(this); }
    static mixin_allocator* from_c_handle(dnmx_mixin_allocator_handle ha) noexcept { return reinterpret_cast<mixin_allocator*>(ha); }
};

}