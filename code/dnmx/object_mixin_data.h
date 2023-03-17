// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <stdint.h>

struct dnmx_basic_object;

// data for a mixin in an object
typedef struct dnmx_object_mixin_data {
    uint8_t* buf; // beginning of buffer
    uint8_t* mixin; // pointer to mixin within buffer

#if defined(__cplusplus)
    void set_object(dnmx_basic_object* obj) noexcept {
        void** obj_ptr_in_buf = reinterpret_cast<void**>(mixin - sizeof(void*));
        *obj_ptr_in_buf = obj;
    }
#endif
} dnmx_object_mixin_data;
