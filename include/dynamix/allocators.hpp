// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Allocator classes.
 */

#include "global.hpp"
#include "object_type_info.hpp"

namespace dynamix
{

/**
 * The class should be the parent to your custom
 * allocators, i.e. allocators that are set
 * for all mixin allocation
 */
class DYNAMIX_API global_allocator
{
public:
    virtual ~global_allocator() {}

    /// Pure virtual.
    /// Should return a valid pointer to an array with the size of \c count
    /// `mixin_data_in_object` instances.
    ///
    /// Use the static constant `mixin_data_size` to get the size of a single
    /// `mixin_data_in_object`
    ///
    /// \par Example:
    /// \code
    /// char* my_allocator::alloc_mixin_data(size_t count)
    /// {
    ///     return new char[count * mixin_data_size];
    /// }
    /// \endcode
    virtual char* alloc_mixin_data(size_t count) = 0;

    /// Pure virtual.
    /// Should free the memory that has
    /// been obtained via a call to `alloc_mixin_data`.
    virtual void dealloc_mixin_data(char* ptr) = 0;

    /// Calculates appropriate size for a mixin buffer
    /// so as to satisfy the requirements of mixin size and alignment
    /// AND leave a room for its owning object in front.
    ///
    /// You may use it in your overrides of `alloc_mixin` to determine
    /// the appropriate memory size.
    static size_t calculate_mem_size_for_mixin(size_t mixin_size, size_t mixin_alignment);

    /// Calculates the appropriate offset of the mixin in the buffer
    /// so as to satisfy the requirements of its alignment
    /// AND leave a room for its owning object in front.
    ///
    /// You may use it in your overrides of `alloc_mixin` to determine
    /// the correct mixin_offset.
    static size_t calculate_mixin_offset(const char* buffer, size_t mixin_alignment);

    /// Pure virtual.
    /// Should return memory for a mixin instance.
    /// The library will request a buffer in which to put the mixin.
    /// Overrides of this method should fill the output parameters with the
    /// address of the allocated memory and the offset of the mixin
    /// (according to the alignment)
    /// BUT IN SUCH A WAY AS TO ALLOW A POINTER TO BE PLACED IN FRONT
    ///
    /// You may use `calculate_mem_size_for_mixin` and `calculate_mixin_offset`
    /// if you're not sure what to do.
    ///
    /// \par Example:
    /// \code
    /// void your_allocator::alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer, size_t& out_mixin_offset)
    /// {
    ///     size_t mem_size = calculate_mem_size_for_mixin(mixin_size, mixin_alignment);
    ///     out_buffer = new char[mem_size];
    ///
    ///     out_mixin_offset = calculate_mixin_offset(out_buffer, mixin_alignment);
    /// }
    /// \endcode
    virtual void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer, size_t& out_mixin_offset) = 0;

    /// Pure virtual.
    /// Should free the memory that has
    /// been obtained via a call to `alloc_mixin`.
    virtual void dealloc_mixin(char* ptr) = 0;

    /// Size of `mixin_data_in_object`
    ///
    /// Use this to determine how many bytes you'll allocate for single
    /// mixin data in `alloc_mixin_data`
    static constexpr size_t mixin_data_size = sizeof(internal::mixin_data_in_object);

#if defined(DYNAMIX_DEBUG)
    // checks to see if an allocator is changed after it has already started allocating
    // it could be a serious bug to allocate from one and deallocate from another

    // users are encouraged to make use of this when debugging
    global_allocator() : _has_allocated(false) {}

    bool has_allocated() const { return _has_allocated; }

protected:
    bool _has_allocated;
#endif
};

/**
 * The class should be the parent to your custom
 * mixin allocators, i.e. allocators that are set to mixins
 * mixin as features.
 *
 * It's derived from `allocator`, and the difference
 * between the two is that `mixin_allocator`, hides `alloc_mixin_data` and
 * `dealloc_mixin_data`.
 */
class DYNAMIX_API mixin_allocator : public global_allocator
{
private:
    /// \internal
    virtual char* alloc_mixin_data(size_t count) override;
    /// \internal
    virtual void dealloc_mixin_data(char* ptr) override;
};

namespace internal
{

/**
 * The default allocator.
 *
 * Used internally by the library, where no custom allocators are provided.
 */
class DYNAMIX_API default_allocator : public global_allocator
{
public:
    /// \internal
    virtual char* alloc_mixin_data(size_t count) override;
    /// \internal
    virtual void dealloc_mixin_data(char* ptr) override;
    /// \internal
    virtual void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer, size_t& out_mixin_offset) override;
    /// \internal
    virtual void dealloc_mixin(char* ptr) override;
};


} // namespace internal

/// Feature list entry function for custom mixin allocators
template <typename CusomAllocator>
mixin_allocator& allocator()
{
    static CusomAllocator alloc;
    return alloc;
}

} // namespace dynamix
