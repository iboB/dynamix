// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"

#if DYNAMIX_HAS_PMR
#include <memory_resource>
namespace dynamix::compat::pmr {
using memory_resouce = std::pmr::memory_resource;
}
#else
#include "../../../dnmx/api.h"
#include <cstddef>

namespace dynamix::compat::pmr {
class DYNAMIX_API memory_resource {
public:
    virtual ~memory_resource();

    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
        return do_allocate(bytes, alignment);
    }
    void deallocate(void* p, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
        do_deallocate(p, bytes, alignment);
    }
    bool is_equal(const memory_resource& other) const noexcept {
        return do_is_equal(other);
    }
protected:
    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;
    virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) = 0;
    virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
};

DYNAMIX_API memory_resource* get_default_resource() noexcept;
}
#endif
