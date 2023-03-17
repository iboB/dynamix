// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "pmr.hpp"

#if DYNAMIX_HAS_PMR
#include <itlib/pmr_allocator.hpp>
namespace dynamix::compat::pmr {
template <typename T = std::byte>
using allocator = itlib::pmr_allocator<T>;
}
#else
#include "memory_resource.hpp"

namespace dynamix::compat::pmr {
template <typename T = std::byte>
class allocator {
    memory_resource& m_resource;
public:
    using value_type = T;

    allocator() noexcept
        : m_resource(*get_default_resource())
    {}
    template<class U>
    allocator(const allocator<U>& other) noexcept
        : m_resource(*other.resource())
    {}
    allocator(memory_resource* r)
        : m_resource(*r)
    {}

    allocator(const allocator&) = default;
    allocator& operator=(const allocator&) = delete;

    memory_resource* resource() const noexcept { return &m_resource; };

    [[nodiscard]] T* allocate(std::size_t n) {
        return static_cast<T*>(m_resource.allocate(n * sizeof(T), alignof(T)));
    }
    void deallocate(T* p, std::size_t n) {
        m_resource.deallocate(p, n * sizeof(T), alignof(T));
    }

    allocator select_on_container_copy_construction() const { return {}; }

    [[nodiscard]] void* allocate_bytes(std::size_t n, std::size_t a = alignof(std::max_align_t)) {
        return m_resource.allocate(n, a);
    }

    void deallocate_bytes(void* p, std::size_t n, std::size_t a = alignof(std::max_align_t)) {
        m_resource.deallocate(p, n, a);
    }

    friend bool operator==(const allocator& lhs, const allocator& rhs) noexcept {
        return lhs.resource() == rhs.resource();
    }
    friend bool operator!=(const allocator& lhs, const allocator& rhs) noexcept {
        return lhs.resource() != rhs.resource();
    }
};

template< class T1, class T2 >
bool operator==(const allocator<T1>& lhs, const allocator<T2>& rhs) noexcept {
    return lhs.resource() == rhs.resource();
}
template< class T1, class T2 >
bool operator!=(const allocator<T1>& lhs, const allocator<T2>& rhs) noexcept {
    return lhs.resource() != rhs.resource();
}
}
#endif
