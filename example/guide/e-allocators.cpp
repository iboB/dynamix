// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/advanced/allocators.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/mixin_info.hpp>
#include <dynamix/mixin_allocator.hpp>
#include <dynamix/compat/pmr/memory_resource.hpp>

#include <iostream>
#include <mutex>
#include <new>
#include <string>

DYNAMIX_DECLARE_DOMAIN(game);

// [classes]
struct transform { float x = 0, y = 0; };
struct health { int hp = 100; };
struct name { std::string value; };
struct script { std::mutex mutex; }; // not movable: external
struct particle { float life = 1; }; // will use a mixin allocator
// [classes]

DYNAMIX_DECLARE_MIXIN(transform);
DYNAMIX_DECLARE_MIXIN(health);
DYNAMIX_DECLARE_MIXIN(name);
DYNAMIX_DECLARE_MIXIN(script);
DYNAMIX_DECLARE_MIXIN(particle);

// [resource]
// a memory resource which counts allocations
class counting_resource : public dynamix::compat::pmr::memory_resource {
public:
    int allocs = 0;
    int deallocs = 0;
protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        ++allocs;
        return ::operator new(bytes, std::align_val_t(alignment));
    }
    void do_deallocate(void* p, std::size_t, std::size_t alignment) override {
        ++deallocs;
        ::operator delete(p, std::align_val_t(alignment));
    }
    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }
};
// [resource]

// [mixin-allocator]
// a mixin allocator: allocates the external buffers of the mixins which use it
class particle_allocator final : public dynamix::mixin_allocator {
public:
    int allocs = 0;
    dynamix::byte_t* alloc_mixin_buf(const dynamix::mixin_info& info) override {
        ++allocs;
        // the buffer must have at least this size and alignment
        // (they include room for the pointer to the object)
        return static_cast<dynamix::byte_t*>(::operator new(info.obj_buf_size,
            std::align_val_t(info.obj_buf_alignment_and_mixin_offset)));
    }
    void dealloc_mixin_buf(dynamix::byte_t* ptr, const dynamix::mixin_info& info) noexcept override {
        ::operator delete(ptr, std::align_val_t(info.obj_buf_alignment_and_mixin_offset));
    }
};
particle_allocator the_particle_allocator;
// [mixin-allocator]

int main() {
    using namespace dynamix;

    // [object]
    counting_resource res;
    {
        object obj(g::get_domain<game>(), allocator(&res));

        // all internal mixins are in a single buffer
        mutate(obj, add<transform>(), add<health>(), add<name>());
        std::cout << "allocations: " << res.allocs << '\n'; // 1

        // a mutation allocates a new buffer
        mutate(obj, remove<name>());
        std::cout << "allocations: " << res.allocs << '\n'; // 2

        // external mixins are allocated separately (by the object's allocator)
        mutate(obj, add<script>());
        std::cout << "allocations: " << res.allocs << '\n'; // 4: new buffer + script
    }
    std::cout << "deallocations: " << res.deallocs << '\n'; // 4
    // [object]

    // [use-mixin-allocator]
    {
        object obj(g::get_domain<game>());
        mutate(obj, add<transform>(), add<particle>());
        std::cout << "particle allocations: " << the_particle_allocator.allocs << '\n'; // 1
    }
    // [use-mixin-allocator]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>

// [define]
DYNAMIX_DEFINE_MIXIN(game, transform);
DYNAMIX_DEFINE_MIXIN(game, health);
DYNAMIX_DEFINE_MIXIN(game, name);
DYNAMIX_DEFINE_MIXIN(game, script);
DYNAMIX_DEFINE_MIXIN(game, particle)
    .uses_allocator(the_particle_allocator)
;
// [define]

DYNAMIX_DEFINE_DOMAIN(game, "game");
