// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/sv.h"
#include "bits/id_int_t.h"
#include "bits/noexcept.h"
#include "type_handle.h"
#include "domain_handle.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dnmx_type_class_id {
    dnmx_id_int_t i;

#if defined(__cplusplus)
    using int_t = dnmx_id_int_t;
    constexpr bool operator==(dnmx_type_class_id o) const noexcept { return i == o.i; }
    constexpr bool operator!=(dnmx_type_class_id o) const noexcept { return i != o.i; }
#endif
} dnmx_type_class_id;

typedef bool (*dnmx_type_class_check_func)(dnmx_type_handle) DNMX_NOEXCEPT;

typedef struct dnmx_type_class {
    dnmx_sv name;
    dnmx_type_class_check_func check;

    // if domain is not null, this means the type class is registered and checks can be
    // made by id and name, instead of the check function
    // if domain is null, this means the type class is not registered and checks can only
    // be made by invoking the check function
    dnmx_domain_handle domain;
    dnmx_type_class_id id;

#if defined(__cplusplus)
    dnmx_id_int_t iid() const noexcept { return id.i; }
#endif
} dnmx_type_class;

#if defined(__cplusplus)
}
#endif
