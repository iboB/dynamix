// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/sv.h"
#include "bits/noexcept.h"
#include "type_handle.h"

typedef bool (*dnmx_type_class_match_func)(dnmx_type_handle) DNMX_NOEXCEPT;

typedef struct dnmx_type_class {
    // may be left empty, but in such a case the type class cannot be registered in a domain
    dnmx_sv name;

    // this must not be left null: it's always assumed to not be null
    dnmx_type_class_match_func matches;
} dnmx_type_class;
