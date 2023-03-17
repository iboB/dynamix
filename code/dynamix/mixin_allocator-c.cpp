// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/mixin_allocator.h"
#include "mixin_allocator.hpp"
#include <new>

// also export vtable from here
namespace dynamix {
mixin_allocator::~mixin_allocator() = default;

namespace {
class c_mixin_allocator final : public mixin_allocator {
public:
    dnmx_alloc_mixin_func m_alloc;
    dnmx_dealloc_mixin_func m_dealloc;
    c_mixin_allocator(dnmx_alloc_mixin_func alloc, dnmx_dealloc_mixin_func dealloc)
        : m_alloc(alloc), m_dealloc(dealloc)
    {}

    byte_t* alloc_mixin_buf(const mixin_info& info) override {
        auto ret = m_alloc(&info);
        if (!ret) throw std::bad_alloc();
        return static_cast<byte_t*>(ret);
    }
    void dealloc_mixin_buf(byte_t* ptr, const mixin_info& info) noexcept override {
        m_dealloc(ptr, &info);
    }
};
}
}

using namespace dynamix;

extern "C" {
dnmx_mixin_allocator_handle dnmx_create_mixin_allocator(dnmx_alloc_mixin_func alloc, dnmx_dealloc_mixin_func dealloc) {
    auto ret = new c_mixin_allocator(alloc, dealloc);
    return ret->to_c_hanlde();;
}
void dnmx_destroy_mixin_allocator(dnmx_mixin_allocator_handle ha) {
    delete mixin_allocator::from_c_handle(ha);
}
}
