// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_payload.hpp"
#include "allocator.hpp"
#include <memory>

namespace dynamix {

// so, how come we use std::shared_ptr<void> as any as opposed to std::any?
// first it allows us to use an allocator when allocating
// this is also true for itlib::any, so why don't we use itlib::any?
// std::shared_ptr has two important features which are critical to the library
// * it's shared, so this means that payloads can be shared
// * it can alias null, which means that payloads can come from the outside world

template <typename T>
using t_any = std::shared_ptr<T>;

using any = std::shared_ptr<void>;

template <typename T>
auto make_any(const allocator& alloc) -> t_any<typename std::remove_reference<T>::type> {
    return std::allocate_shared<typename std::remove_reference<T>::type>(alloc);
}

template <typename T>
auto make_any(const allocator& alloc, T&& t) -> t_any<typename std::remove_reference<T>::type> {
    return std::allocate_shared<typename std::remove_reference<T>::type>(alloc, std::forward<T>(t));
}

template <typename T>
auto fwd_any(T* ptr) noexcept -> t_any<T> {
    return t_any<T>(any{}, ptr);
}

template <typename T>
auto fwd_any(T& ref) noexcept -> t_any<T> {
    return t_any<T>(any{}, &ref);
}

}
