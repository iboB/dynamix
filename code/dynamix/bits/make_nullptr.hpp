// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace dynamix::impl {
template <typename T>
T* make_nullptr() noexcept { return static_cast<T*>(nullptr); }
}
