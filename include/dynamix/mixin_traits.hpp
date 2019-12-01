// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"

#include "mixin_type_info.hpp"

#include <type_traits>
#include <utility>

namespace dynamix
{

namespace internal
{

// getters of type info based on typeid
#if DYNAMIX_USE_TYPEID
    extern DYNAMIX_API const char* get_mixin_name_from_typeid(const char* typeid_name);
#   if defined(__GNUC__)
    extern DYNAMIX_API void free_mixin_name_from_typeid(const char* typeid_name);
#   endif
#endif

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
    mixin_type_info::mixin_copy_proc>::type get_mixin_copy_constructor()
{
    return call_mixin_copy_constructor<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_copy_constructible<Mixin>::value,
    mixin_type_info::mixin_copy_proc>::type get_mixin_copy_constructor()
{
    return nullptr;
}

template <typename Mixin>
typename std::enable_if<std::is_copy_assignable<Mixin>::value,
    mixin_type_info::mixin_copy_proc>::type get_mixin_copy_assignment()
{
    return call_mixin_copy_assignment<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_copy_assignable<Mixin>::value,
    mixin_type_info::mixin_copy_proc>::type get_mixin_copy_assignment()
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
    mixin_type_info::mixin_move_proc>::type get_mixin_move_constructor()
{
    return call_mixin_move_constructor<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_move_constructible<Mixin>::value,
    mixin_type_info::mixin_move_proc>::type get_mixin_move_constructor()
{
    return nullptr;
}

template <typename Mixin>
void call_mixin_move_assignment(void* target, void* source)
{
    *reinterpret_cast<Mixin*>(target) = std::move(*reinterpret_cast<Mixin*>(source));
}

template <typename Mixin>
typename std::enable_if<std::is_move_assignable<Mixin>::value,
    mixin_type_info::mixin_move_proc>::type get_mixin_move_assignment()
{
    return call_mixin_move_assignment<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_move_assignable<Mixin>::value,
    mixin_type_info::mixin_move_proc>::type get_mixin_move_assignment()
{
    return nullptr;
}

// set any traits which are not already set with a meaningful default
template <typename Mixin>
void set_missing_traits_to_info(mixin_type_info& info)
{
    if (!info.size) info.size = sizeof(Mixin);
    if (!info.alignment) info.alignment = std::alignment_of<Mixin>::value;
    if (!info.constructor) info.constructor = &call_mixin_constructor<Mixin>;
    if (!info.destructor) info.destructor = &call_mixin_destructor<Mixin>;
    if (!info.copy_constructor) info.copy_constructor = get_mixin_copy_constructor<Mixin>();
    if (!info.copy_assignment) info.copy_assignment = get_mixin_copy_assignment<Mixin>();
    if (!info.move_constructor) info.move_constructor = get_mixin_move_constructor<Mixin>();
    if (!info.move_assignment) info.move_assignment = get_mixin_move_assignment<Mixin>();

    if (!info.name)
    {
#if DYNAMIX_USE_TYPEID
        info.name = get_mixin_name_from_typeid(typeid(Mixin).name());
#   if defined(__GNUC__)
        info.owns_name = true;
#   endif
#elif DYNAMIX_USE_STATIC_MEMBER_NAME
        // defining DYNAMIX_USE_STATIC_MEMBER_NAME means that you must provide
        // mixin names with a static const char* member function
        info.name = Mixin::dynamix_mixin_name();
#endif
    }
}

} // namespace internal
}