// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "size.hpp"

namespace dynamix {
class object;

// get object of mixin by mixin ptr
// it's the responsibility of the user to provide a valid mixin address here
// (that is a mixin which is part of an object)
// not doing so leads to UB
inline object* object_of(void* mixin_ptr) {
    return *reinterpret_cast<object**>(reinterpret_cast<char*>(mixin_ptr) - sizeof_ptr);
}
inline const object* object_of(const void* mixin_ptr) {
    return *reinterpret_cast<const object* const*>(reinterpret_cast<const char*>(mixin_ptr) - sizeof_ptr);
}

}
