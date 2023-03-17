// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "domain_settings.h"
#include "bits/sv.h"

#include <stdint.h>

struct dnmx_basic_domain {
    // the values here are visible to C to make debugging easier

    // this value is not used by the library but can be helpful when debugging multiple domains
    dnmx_sv m_name;

    dnmx_domain_settings m_settings;

    // a user data which accessible wherever the domain is
    // the library does not touch this value
    uintptr_t user_data;

    // a context to the domain. this is an additional user data
    // the library does not touch this value
    void* context;
};
