// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "memory_resource.hpp"
#if !DYNAMIX_HAS_PMR // no else (using std::pmr)
#include "../../alloc_util.hpp"
#include <cstdlib>
#include <new>

namespace dynamix::compat::pmr {
memory_resource::~memory_resource() = default; // export vtable

namespace {
class default_resource final : public memory_resource {
    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) override {
#if defined(_MSC_VER)
        void* ret = _aligned_malloc(bytes, alignment);
#else
#if defined(__APPLE__)
        // apple's implementation of aligned_alloc seems to return nullptr with alignments less than sizeof_ptr
        // ... oh, apple...
        if (alignment < sizeof_ptr) alignment = sizeof_ptr;
#endif
        // aligned_alloc requires size to be a multiple of alignment
        auto size = dynamix::util::next_multiple(bytes, alignment);
        void* ret = aligned_alloc(alignment, size);
#endif
        if (!ret) throw std::bad_alloc{};
        return ret;

    }
    virtual void do_deallocate(void* ptr, std::size_t, std::size_t) override {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        free(ptr);
#endif

    }
    virtual bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }
};
}

memory_resource* get_default_resource() noexcept {
    static default_resource res;
    return &res;
}
}

#endif
