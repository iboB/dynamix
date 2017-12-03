// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"

#include <dynamix/allocators.hpp>

#if DYNAMIX_DEBUG
#include <dynamix/object.hpp>
#endif

namespace dynamix
{

// ceil(a/b)*b with integers
// scales a so an exact number of b will fit in it
static size_t ceil_scale(size_t a, size_t b)
{
    size_t result = (a+b-1)/b;
    result*=b;

    return result;
}

size_t domain_allocator::calculate_mem_size_for_mixin(size_t mixin_size, size_t mixin_alignment)
{
    // normally alignof(x) + sizeof(x) is enough for an aligned allocation
    // but in this case we want to have an object* before that and the alignment
    // could be smaller than sizeof(object*) - especially on 64 bit platforms
    size_t mem_size = ceil_scale(sizeof(object*), mixin_alignment);
    mem_size += mixin_size;

    // now it could be the case that the mixin alignment doesn't match the pointer alignment
    // and allocations from a consecutive allocator may end my misaligning the memory for
    // our object* pointer
    // the case could be such if the mixin class doesn't have any data members or has data members,
    // smaller than uintptr_t
    // so, we perform another division rounding up for the final memory size
    mem_size = ceil_scale(mem_size, sizeof(object*));

    return mem_size;
}

size_t domain_allocator::calculate_mixin_offset(const char* buffer, size_t mixin_alignment)
{
    // now malloc (or new) should make sure to give us memory that's word aligned
    // that means that buffer should be aligned to sizeof(ptr)

    // WARNING: if you don't have a custom allocator and this assert fails
    // this means that memory not-aligned to the pointer size was allocated
    // This platform is strange and creepy and is not supported by the default allocator
    // you should write your own, that allocates properly aligned memory
    DYNAMIX_ASSERT_MSG(uintptr_t(buffer) % sizeof(object*) == 0,
        "allocators should always return memory aligned to sizeof(void*)");

    uintptr_t mixin_pos = ceil_scale(uintptr_t(buffer + sizeof(object*)), mixin_alignment);

    return mixin_pos - uintptr_t(buffer);
}


static inline char* allocate_mixin_data(size_t count)
{
    DYNAMIX_ASSERT(domain_allocator::mixin_data_size == sizeof(internal::mixin_data_in_object));
    return new char[sizeof(internal::mixin_data_in_object) * count];
}

static inline void deallocate_mixin_data(char* ptr)
{
    delete[] ptr;
}

char* mixin_allocator::alloc_mixin_data(size_t count, const object*)
{
    DYNAMIX_ASSERT(false); // a mixin allocator should never have to allocate mixin data
    return allocate_mixin_data(count);
}

void mixin_allocator::dealloc_mixin_data(char* ptr, size_t, const object*)
{
    DYNAMIX_ASSERT(false); // a mixin allocator should never have do deallocate mixin data
    deallocate_mixin_data(ptr);
}

void object_allocator::on_set_to_object(object& owner)
{}

void object_allocator::release() noexcept
{}

object_allocator* object_allocator::on_copy_construct(object& target, const object& source)
{
    DYNAMIX_ASSERT(source.allocator() == this);
    return nullptr;
}

object_allocator* object_allocator::on_move(object& target, object& source) noexcept
{
    DYNAMIX_ASSERT(source.allocator() == this);
    return this;
}

namespace internal
{

char* default_allocator::alloc_mixin_data(size_t count, const object*)
{
#if DYNAMIX_DEBUG
    _has_allocated = true;
#endif
    return allocate_mixin_data(count);
}

void default_allocator::dealloc_mixin_data(char* ptr, size_t, const object*)
{
#if DYNAMIX_DEBUG
    DYNAMIX_ASSERT(_has_allocated); // what? deallocate without ever allocating?
#endif
    deallocate_mixin_data(ptr);
}

std::pair<char*, size_t> default_allocator::alloc_mixin(const basic_mixin_type_info& info, const object*)
{
#if DYNAMIX_DEBUG
    _has_allocated = true;
#endif

    size_t mem_size = calculate_mem_size_for_mixin(info.size, info.alignment);

    auto buffer = new char[mem_size];
    auto mixin_offset = calculate_mixin_offset(buffer, info.alignment);

    DYNAMIX_ASSERT(mixin_offset + info.size <= mem_size); // we should have room for the mixin

    return std::make_pair(buffer, mixin_offset);
}

void default_allocator::dealloc_mixin(char* ptr, size_t, const basic_mixin_type_info&, const object*)
{
#if DYNAMIX_DEBUG
    DYNAMIX_ASSERT(_has_allocated); // what? deallocate without ever allocating?
#endif
    delete[] ptr;
}

} // namespace internal

} // namespace dynamix
