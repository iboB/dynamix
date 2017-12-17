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

#include <utility>

namespace dynamix
{

/**
* This class should be the parent to your custom
* mixin allocators, i.e. allocators that are set to mixins
* as features.
*/
class DYNAMIX_API mixin_allocator
{
public:
    virtual ~mixin_allocator() {}

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
    /// Returns a buffer of memory and the offset of the mixin within it
    /// (according to the alignment)
    /// BUT IN SUCH A WAY AS TO ALLOW A POINTER TO BE PLACED IN FRONT
    ///
    /// You may use `calculate_mem_size_for_mixin` and `calculate_mixin_offset`
    /// if you're not sure what to do.
    ///
    /// \par Example:
    /// \code
    /// std::pair<char*, size_t> your_allocator::alloc_mixin(const basic_mixin_type_info& info, const object*)
    /// {
    ///     size_t mem_size = calculate_mem_size_for_mixin(info.size, info.alignment);
    ///     auto buffer = new char[mem_size];
    ///     return make_pair(buffer, calculate_mixin_offset(buffer, info.alignment));
    /// }
    /// \endcode
    virtual std::pair<char*, size_t> alloc_mixin(const basic_mixin_type_info& info, const object* obj) = 0;

    /// Pure virtual.
    /// Should free the memory that has
    /// been obtained via a call to `alloc_mixin`.
    /// The library will call the method with the same mixin type info which was used to allocate it previously
    /// and also with the offset which was returned by the allocation.
    virtual void dealloc_mixin(char* ptr, size_t mixin_offset, const basic_mixin_type_info& info, const object* obj) = 0;

    /// Virtual function, which constructs a mixin within a memory buffer.
    /// The default implementation calls the default constructor.
    virtual void construct_mixin(const basic_mixin_type_info& info, void* ptr);

    /// Virtual function, which copy-constructs a mixin within a memory buffer, from a given source.
    /// Should return false if the copy-construction failed.
    /// The default implementation calls the default copy constructor and returns false if none exists.
    virtual bool copy_construct_mixin(const basic_mixin_type_info& info, void* ptr, const void* source);

    /// Virtual function, which destroys a mixin from a given buffer.
    /// The default implementation calls the destructor.
    virtual void destroy_mixin(const basic_mixin_type_info& info, void* ptr) noexcept;

#if DYNAMIX_DEBUG
    // checks to see if an allocator is changed after it has already started allocating
    // it could be a serious bug to allocate from one and deallocate from another

    // users are encouraged to make use of this when debugging
    mixin_allocator() : _has_allocated(false) {}

    bool has_allocated() const { return _has_allocated; }

protected:
    bool _has_allocated;
#endif
};

/**
* This class is a domain allocator. Inherit from it so you can set
* your custom allocator to the domain.
*/
class DYNAMIX_API domain_allocator : public mixin_allocator
{
public:

    /// Pure virtual.
    /// Should return a valid pointer to an array with the size of \c count
    /// `mixin_data_in_object` instances.
    ///
    /// Use the static constant `mixin_data_size` to get the size of a single
    /// `mixin_data_in_object`
    ///
    /// \par Example:
    /// \code
    /// char* my_allocator::alloc_mixin_data(size_t count, const object*)
    /// {
    ///     return new char[count * mixin_data_size];
    /// }
    /// \endcode
    virtual char* alloc_mixin_data(size_t count, const object* obj) = 0;

    /// Pure virtual.
    /// Should free the memory that has
    /// been obtained via a call to `alloc_mixin_data`.
    /// The number of elements to dealocate will correspond to the number of
    /// elements used to allocated the buffer
    virtual void dealloc_mixin_data(char* ptr, size_t count, const object* obj) = 0;


    /// Size of `mixin_data_in_object`
    ///
    /// Use this to determine how many bytes you'll allocate for single
    /// mixin data in `alloc_mixin_data`
    static constexpr size_t mixin_data_size = sizeof(internal::mixin_data_in_object);
};

/**
* The class should be the parent to your custom
* object allocators, i.e. allocators that are set to objects.
*
* It's derived from `domain_allocator` and provides a number of
* additional virtual methods a user can override.
*/
class DYNAMIX_API object_allocator : public domain_allocator
{
public:
    /// Called when an allocator is set to an object.
    /// This happens in the following cases:
    /// * When the object is constructed with an allocator
    /// * When the object is copy-constructed from another object and
    /// `on_copy_construct` returns non-null
    /// * When an object is moved onto another and `on_move` return non-null
    /// (This could lead to `on_set_to_object` being called multiple times
    /// for the same allocator and different objects)
    ///
    /// The default implementation is empty
    virtual void on_set_to_object(object& owner);

    /// Override this if you want to provide some custom release logic
    /// to an object allocator. It will be called when the object allocator
    /// for a given object should logically be destroyed.
    ///
    /// If multiple objects have the same allocator, this would be a good
    /// spot to decrement a reference counter.
    ///
    /// The default implementation is empty
    virtual void release(object& owner) noexcept;

    /// Called when an object is copy-constructed from the owner object
    /// Use it to provide custom logic so as to create an allocator for the target
    ///
    /// WARNING: Cases which count as copy construction are also ones where
    /// an object is copied onto an empty one. This can happen with `object::copy_from`
    /// or the equality operator if it exists
    ///
    /// The default implementation returns nullptr
    virtual object_allocator* on_copy_construct(object& target, const object& source);

    /// Called when an object is moved from the owner object
    /// Use it to provide custom logic so as to create an allocator for the terget.
    ///
    /// After this function is called the sources allocator will be set to nullptr
    /// *without* `release` being called for it. If you want to return a different allocator
    /// for the target object, you need to release the one from source here.
    ///
    /// The default implementation returns `this` (ie the allocator of `source`)
    virtual object_allocator* on_move(object& target, object& source) noexcept;
};

namespace internal
{

/**
 * The default allocator.
 *
 * Used internally by the library, where no custom allocators are provided.
 */
class DYNAMIX_API default_allocator : public domain_allocator
{
public:
    /// \internal
    virtual char* alloc_mixin_data(size_t count, const object* obj) override;
    /// \internal
    virtual void dealloc_mixin_data(char* ptr, size_t count, const object* obj) override;
    /// \internal
    virtual std::pair<char*, size_t> alloc_mixin(const basic_mixin_type_info& info, const object* obj) override;
    /// \internal
    virtual void dealloc_mixin(char* ptr, size_t mixin_offset, const basic_mixin_type_info& info, const object* obj) override;
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
