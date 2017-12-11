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
 * Defines the internal classes that describe the information about a mixin -
 * features and message data, type name, size, alignment, etc.
 */

#include "global.hpp"
#include "message.hpp"
#include <type_traits>
#include <utility>

namespace dynamix
{

class domain_allocator;

static const mixin_id INVALID_MIXIN_ID = ~mixin_id(0);

/**
* Public mixin type info. Contains a slice of the type info data
* which can be viewed by the library's users
* for all mixin allocation
*/
class DYNAMIX_API basic_mixin_type_info : public internal::noncopyable
{
public:
    /// The mixin's id
    mixin_id id;

    /// Shows whether this is as initialized mixin type
    bool is_valid() const { return id != INVALID_MIXIN_ID; }

    /// The mixin name: The class name or, in case `DYNAMIX_USE_TYPEID` is false,
    /// the name returned from `dynamix_mixin_name`.
    const char* name;

    /// Size of the mixin type
    size_t size;

    /// Alignment of the mixin type
    size_t alignment;

    /// Allocator associated with this mixin type.
    /// If no special one was provided this will be equal to the allocator of the domain.
    domain_allocator* allocator;

#if DYNAMIX_ADDITIONAL_METRICS
    /// Number of "living" mixins of this type.
    mutable size_t num_mixins;
#endif

protected:
    // users shouldn't be able to construct or destroy this

    basic_mixin_type_info(mixin_id i)
        : id(i)
        // since this is always static, other members will be initialized with 0
    {}
    ~basic_mixin_type_info() {}
};

namespace internal
{

typedef void (*mixin_constructor_proc)(void* memory);
typedef void (*mixin_copy_proc)(void* memory, const void* source);
typedef void (*mixin_move_proc)(void* memory, void* source);
typedef void (*mixin_destructor_proc)(void* memory);

// this struct contains information for a given mixin
class DYNAMIX_API mixin_type_info : public basic_mixin_type_info
{
public:

    // procedures, obtained from the mixin definition that makes the actual
    // construction and destruction
    mixin_constructor_proc constructor;
    mixin_destructor_proc destructor;

    // might be left null for mixins which aren't copy-constructible
    mixin_copy_proc copy_constructor;

    // might be left null for mixins which aren't copy-assignable
    mixin_copy_proc copy_assignment;

    // might be left null for mixin which aren't move-constructible
    mixin_move_proc move_constructor;

    // list of all the message infos for the messages this mixin supports
    std::vector<message_for_mixin> message_infos;

    mixin_type_info()
        : basic_mixin_type_info(INVALID_MIXIN_ID)
        // since this is always static, other members will be initialized with 0
    {
    }
};

// this metafunction binds the type info of a mixin to its type
template <typename Mixin>
struct mixin_type_info_instance : public noncopyable
{
    // have this static function instead of a simple member to guarantee
    // that mixin_type_info's constructor is called the first time
    static mixin_type_info& info()
    {
        static mixin_type_info d;
        return d;
    }

    // this static member registers the mixin in the domain
    // we need to reference it somewhere so as to call its constructor
    static mixin_type_info_instance registrator;

    // the constructor is defined in mixin.h because it references the domain object
    mixin_type_info_instance();

    ~mixin_type_info_instance();

    // to prevent warnings and optimizations that will say that we're not using
    // mixin_type_info_instance by simply referencing it
    int unused;
};
template <typename Mixin>
mixin_type_info_instance<Mixin> mixin_type_info_instance<Mixin>::registrator;

// this procedure is used for the mixin construction
template <typename Mixin>
void call_mixin_constructor(void* memory)
{
    new (memory) Mixin;
}

template <typename Mixin>
void call_mixin_destructor(void* memory)
{
    reinterpret_cast<Mixin*>(memory)->~Mixin();
}

template <typename Mixin>
void call_mixin_copy_constructor(void* memory, const void* source)
{
    new (memory) Mixin(*reinterpret_cast<const Mixin*>(source));
}

template <typename Mixin>
void call_mixin_copy_assignment(void* target, const void* source)
{
    *reinterpret_cast<Mixin*>(target) = *reinterpret_cast<const Mixin*>(source);
}

template <typename Mixin>
typename std::enable_if<std::is_copy_constructible<Mixin>::value,
    mixin_copy_proc>::type get_mixin_copy_constructor()
{
    return call_mixin_copy_constructor<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_copy_constructible<Mixin>::value,
    mixin_copy_proc>::type get_mixin_copy_constructor()
{
    return nullptr;
}

template <typename Mixin>
typename std::enable_if<std::is_copy_assignable<Mixin>::value,
    mixin_copy_proc>::type get_mixin_copy_assignment()
{
    return call_mixin_copy_assignment<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_copy_assignable<Mixin>::value,
    mixin_copy_proc>::type get_mixin_copy_assignment()
{
    return nullptr;
}

template <typename Mixin>
void call_mixin_move_constructor(void* memory, void* source)
{
    new (memory) Mixin(std::move(*reinterpret_cast<Mixin*>(source)));
}

template <typename Mixin>
typename std::enable_if<std::is_move_constructible<Mixin>::value,
    mixin_move_proc>::type get_mixin_move_constructor()
{
    return call_mixin_move_constructor<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_move_constructible<Mixin>::value,
    mixin_move_proc>::type get_mixin_move_constructor()
{
    return nullptr;
}

} // namespace internal
} // namespace dynamix
