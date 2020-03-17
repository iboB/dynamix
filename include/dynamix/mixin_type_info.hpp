// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once


/**
 * \file
 * Defines the internal classes that describe the information about a mixin -
 * features and message data, type name, size, alignment, etc.
 */

#include "config.hpp"
#include "mixin_id.hpp"
#include "message.hpp"
#include "metrics.hpp"

#include <utility>
#include <vector>
#include <cstdint>

namespace dynamix
{

class mixin_allocator;

// TODO: inline when on C++17
static constexpr mixin_id INVALID_MIXIN_ID = ~mixin_id(0);

/**
* Mixin type info. Contains a the mixin type information, features and traits.
*/
class DYNAMIX_API mixin_type_info
{
public:
    mixin_type_info() = default;

    typedef void(*mixin_constructor_proc)(void* memory);
    typedef void(*mixin_copy_proc)(void* memory, const void* source);
    typedef void(*mixin_move_proc)(void* memory, void* source);
    typedef void(*mixin_destructor_proc)(void* memory);

    /// The mixin's id
    mixin_id id = INVALID_MIXIN_ID;

    /// Shows whether this is as initialized mixin type
    bool is_valid() const { return id != INVALID_MIXIN_ID; }

    /// The mixin name: The class name or, in case `mixin_name` feature is provided,
    /// the manual name set from there
    const char* name = nullptr;

    /// Size of the mixin type
    size_t size = 0;

    /// Alignment of the mixin type
    size_t alignment = 0;

    /// Allocator associated with this mixin type.
    /// If no special one was provided this will be equal to the allocator of the domain.
    mixin_allocator* allocator = nullptr;

    /// Procedure which calls the default constructor of a mixin.
    /// Might be left null if the mixin is not default-constructible.
    mixin_constructor_proc constructor = 0;

    /// Procedure which calls the destructor of a mixin. Never null.
    mixin_destructor_proc destructor = 0;

    /// Procedutre which calls the copy-constructor of a mixin.
    /// Might be left null for mixins which aren't copy-constructible
    mixin_copy_proc copy_constructor = 0;

    /// Procedure which calls the copy assignment of a mixin
    /// Might be left null for mixins which aren't copy-assignable
    mixin_copy_proc copy_assignment = 0;

    /// Procedure which calls the move-constrcutor of a mixin
    /// Might be left null for mixin which aren't move-constructible
    mixin_move_proc move_constructor = 0;

    /// Procedure which calls the move assignment of a mixin
    /// Might be left null for mixin which aren't move-constructible
    mixin_move_proc move_assignment = 0;

    /// All the message infos for the messages this mixin supports
    std::vector<internal::message_for_mixin> message_infos;

    /// User data associated with this type info
    uintptr_t user_data = 0;

#if DYNAMIX_USE_TYPEID && defined(__GNUC__)
    // boolean which shows whether the name in the mixin type info was obtained
    // by cxa demangle and should be freed
    bool owns_name = false;
#endif

    /// Number of "living" mixins of this type.
    /// Note that this is a counter of mixins which are within objects and not
    /// a counter of instances which you might otherwise have
    mutable metric num_mixins = {size_t(0)};

    // non-copyable
    mixin_type_info(const mixin_type_info&) = delete;
    mixin_type_info& operator=(const mixin_type_info&) = delete;

    // non-movable
    mixin_type_info(mixin_type_info&&) = delete;
    mixin_type_info& operator=(mixin_type_info&&) = delete;
};

} // namespace dynamix
