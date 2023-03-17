// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/mixin_allocator.hpp>
#include <dynamix/mixin_info.hpp>
#include <dynamix/alloc_util.hpp>
#include <string>
#include <cstdlib>
#include <stdexcept>

class test_mixin_allocator final : public dynamix::mixin_allocator {
public:
    virtual dynamix::byte_t* alloc_mixin_buf(const dynamix::mixin_info& info) override {
        if (info.user_data == 666) throw std::logic_error("bad alloc");

#if defined(_MSC_VER)
        void* ret = _aligned_malloc(info.obj_buf_size, info.obj_buf_alignment_and_mixin_offset);
#else
        // aligned_alloc requires size to be a multiple of alignment
        auto size = dynamix::util::next_multiple(info.obj_buf_size, info.obj_buf_alignment_and_mixin_offset);
        void* ret = aligned_alloc(info.obj_buf_alignment_and_mixin_offset, size);
#endif
        return static_cast<dynamix::byte_t*>(ret);
    }
    virtual void dealloc_mixin_buf(dynamix::byte_t* ptr, const dynamix::mixin_info&) noexcept override {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
};
