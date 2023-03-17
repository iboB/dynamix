// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <new>
#include <tuple>
#include <type_traits>

// more or less a reimplementation of std::make_from_tuple but with placement new
namespace dynamix::impl {
template <typename T, typename Tuple, std::size_t... I>
void _make_from_tuple_impl(void* ptr, Tuple&& tup, std::index_sequence<I...>)
{
    static_assert(std::is_constructible_v<T, decltype(std::get<I>(std::declval<Tuple>()))...>);
    new (ptr) T(std::get<I>(std::forward<Tuple>(tup))...);
}

template <typename T, typename Tuple>
void make_from_tuple(void* ptr, Tuple&& tup) {
    _make_from_tuple_impl<T>(ptr, std::forward<Tuple>(tup),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>());
}
}
