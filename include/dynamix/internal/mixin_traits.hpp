// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "../config.hpp"

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

template <typename Mixin>
void call_mixin_move_assignment(void* target, void* source)
{
    *reinterpret_cast<Mixin*>(target) = std::move(*reinterpret_cast<Mixin*>(source));
}

template <typename Mixin>
typename std::enable_if<std::is_move_assignable<Mixin>::value,
    mixin_move_proc>::type get_mixin_move_assignment()
{
    return call_mixin_move_assignment<Mixin>;
}

template <typename Mixin>
typename std::enable_if<!std::is_move_assignable<Mixin>::value,
    mixin_move_proc>::type get_mixin_move_assignment()
{
    return nullptr;
}

} // namespace internal
}