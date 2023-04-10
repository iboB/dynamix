// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "mixin_info_fwd.hpp"
#include "mixin_index.hpp"
#include "size.hpp"

namespace dynamix {
class type;
struct init_new_args {
    const mixin_info& info;
    byte_t* mixin_buf; // buffer of mixin to be initialized or updated (never null)
    const type& target_type;
    mixin_index_t target_index;
};

struct update_common_args : init_new_args {
    const type& source_type;
    mixin_index_t source_index;
};

namespace util {
DYNAMIX_API void default_init_new_func(init_new_args args); // defined in object_mutation.cpp
inline void noop_udpate_common_func(update_common_args) {}
}
}
