# Allocators and Internal/External Mixins

DynaMix allows controlling all of its allocations. This is useful for performance (cache locality, avoiding the general-purpose heap), for tracking memory usage, and for environments with special memory requirements.

> [!TIP]
> The code on this page is from [example/guide/allocators.cpp](../../example/guide/allocators.cpp). It's a complete program which is built and run with the library's tests.

## Contents

* [Where DynaMix allocates](#where-dynamix-allocates)
* [Polymorphic allocators](#polymorphic-allocators)
* [Internal and external mixins](#internal-and-external-mixins)
* [Mixin allocators](#mixin-allocators)

## Where DynaMix allocates

* **Domains** allocate their registries, types, and the cache of type queries. This happens when mixins are registered and when new types are created, which is typically early in a program's life, and is rare afterwards.
* **Objects** allocate a buffer for their mixins. Each mutation allocates a new buffer and deallocates the old one.
* **External mixins** are allocated separately from their object's buffer.

All of these use *polymorphic allocators*, except external mixins which can optionally use a *mixin allocator*.

Message calls never allocate.

## Polymorphic allocators

`dynamix::allocator` is a polymorphic allocator: `std::pmr::polymorphic_allocator`, or a compatible implementation where the standard library doesn't provide it (see [compat/pmr](../../code/dynamix/compat/pmr/README.md)). It's created from a pointer to a `memory_resource`. The default one uses the default memory resource: `new` and `delete`.

Domains and objects take an allocator as an optional constructor argument:

```c++
dynamix::domain dom("name", settings, user_data, context, alloc);
dynamix::object obj(dom, alloc);
dynamix::object obj2(type, alloc);
auto copy = obj.copy(alloc);
```

For domains defined with `DYNAMIX_DEFINE_DOMAIN`, the allocator is the last argument of the macro.

Here's a memory resource which counts allocations, used with an object:

```c++
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
```

```c++
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
```

Use `dynamix::compat::pmr::memory_resource` as the base class of custom resources, as above. It's `std::pmr::memory_resource` where available, so standard resources like `std::pmr::monotonic_buffer_resource` and `std::pmr::unsynchronized_pool_resource` can be used directly in that case.

A common use is to allocate objects which are created and destroyed together, like the objects of a level in a game, from a monotonic or pool resource.

The allocator of an object can't be changed. It's copied with the object when it's moved, but not when it's copied (`copy` takes an allocator for the new object). Move assignment between objects with different allocators throws `object_error`.

## Internal and external mixins

*Internal* mixins are allocated in their object's buffer, which also contains the per-mixin data of the object. An object with only internal mixins makes a single allocation, and its mixins are next to each other in memory. This is the default and it's preferable for performance.

*External* mixins are allocated separately, and only a pointer to them is in the object's buffer. A mixin is external if:

* it's not nothrow move constructible (since internal mixins are moved when their object is mutated),
* it has a mixin allocator (see below),
* or it's explicitly marked with `.force_external()` in its definition (or `force_external` in its `mixin_info`).

The trade-offs:

| | Internal | External |
|---|---|---|
| Allocations | One per object | One more per mixin |
| Cache locality | Good: in the object buffer | Worse: separate memory |
| Moved on mutations | Yes: pointers to them are invalidated | No: pointers to them are stable |
| Requirements | Nothrow move constructor | None |

Make a mixin external if it needs a stable address (for example if pointers to it are held by other systems), or if it's large and objects are mutated often, so that it's not moved with every mutation.

## Mixin allocators

A mixin allocator allocates the external buffers of specific mixins. It's a class derived from [`dynamix::mixin_allocator`](../../code/dynamix/mixin_allocator.hpp):

```c++
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
```

```c++
DYNAMIX_DEFINE_MIXIN(game, particle)
    .uses_allocator(the_particle_allocator)
;
```

The buffer must have the size `info.obj_buf_size` and the alignment `info.obj_buf_alignment_and_mixin_offset`. It's bigger than the mixin, because a pointer to the object is stored before it (this is how [`object_of`](../cpp/dm-this.md#object_of) works).

A mixin allocator is used for all instances of the mixin, regardless of the allocators of their objects. It's useful for mixins which are created in large numbers, like particles or bullets, where a pool allocator would be faster and use less memory than the general-purpose heap.

The builder has three ways to set a mixin allocator:

* `.uses_allocator(alloc)` - by reference. The allocator must outlive the mixin's registration.
* `.uses_allocator<Alloc>()` - a new instance of `Alloc` is created and owned by the mixin's definition.
* `.uses_allocator(t_any<mixin_allocator>)` - any other ownership.

In C, mixin allocators are created with `dnmx_create_mixin_allocator` from a pair of functions.
