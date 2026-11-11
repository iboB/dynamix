// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/compat/pmr/memory_resource.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <new>
#include <utility>
#include <vector>

// simple memory resources for benchmarks
// we don't use the standard ones (std::pmr::monotonic_buffer_resource, etc.), as they're not available
// on all platforms which DynaMix supports (DynaMix has a pmr compatibility layer for them)
namespace dbench {

// allocates from big blocks with a bump pointer, never deallocates
// this is the fastest possible allocation strategy and is a good fit for objects
// which are created and destroyed together
class arena_resource final : public dynamix::compat::pmr::memory_resource {
public:
    explicit arena_resource(size_t block_size = 1024 * 1024) : m_block_size(block_size) {}

    arena_resource(const arena_resource&) = delete;
    arena_resource& operator=(const arena_resource&) = delete;

private:
    void* do_allocate(size_t bytes, size_t alignment) override {
        auto aligned = (m_pos + alignment - 1) & ~(uintptr_t(alignment) - 1);
        if (m_blocks.empty() || aligned + bytes > m_end) {
            auto size = bytes + alignment > m_block_size ? bytes + alignment : m_block_size;
            m_blocks.emplace_back(new std::byte[size]);
            m_pos = reinterpret_cast<uintptr_t>(m_blocks.back().get());
            m_end = m_pos + size;
            aligned = (m_pos + alignment - 1) & ~(uintptr_t(alignment) - 1);
        }
        m_pos = aligned + bytes;
        return reinterpret_cast<void*>(aligned);
    }
    void do_deallocate(void*, size_t, size_t) override {}
    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }

    size_t m_block_size;
    std::vector<std::unique_ptr<std::byte[]>> m_blocks;
    uintptr_t m_pos = 0;
    uintptr_t m_end = 0;
};

// keeps deallocated buffers in free lists per size and alignment, and reuses them
// a simplified version of a pool resource: good for objects which are mutated often,
// since a mutation deallocates a buffer and allocates one of a different size
class recycling_resource final : public dynamix::compat::pmr::memory_resource {
public:
    recycling_resource() = default;
    ~recycling_resource() {
        for (auto& [key, list] : m_free) {
            for (auto p : list) {
                ::operator delete(p, std::align_val_t(key.second));
            }
        }
    }

    recycling_resource(const recycling_resource&) = delete;
    recycling_resource& operator=(const recycling_resource&) = delete;

private:
    using key = std::pair<size_t, size_t>; // size, alignment

    void* do_allocate(size_t bytes, size_t alignment) override {
        auto f = m_free.find({bytes, alignment});
        if (f != m_free.end() && !f->second.empty()) {
            auto ret = f->second.back();
            f->second.pop_back();
            return ret;
        }
        return ::operator new(bytes, std::align_val_t(alignment));
    }
    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        m_free[{bytes, alignment}].push_back(p);
    }
    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }

    std::map<key, std::vector<void*>> m_free;
};

// counts allocations and allocated bytes
class counting_resource final : public dynamix::compat::pmr::memory_resource {
public:
    size_t allocs = 0;
    size_t deallocs = 0;
    size_t bytes = 0; // currently allocated

    void reset_counters() {
        allocs = 0;
        deallocs = 0;
    }

private:
    void* do_allocate(size_t b, size_t alignment) override {
        ++allocs;
        bytes += b;
        return ::operator new(b, std::align_val_t(alignment));
    }
    void do_deallocate(void* p, size_t b, size_t alignment) override {
        ++deallocs;
        bytes -= b;
        ::operator delete(p, std::align_val_t(alignment));
    }
    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }
};

} // namespace dbench
