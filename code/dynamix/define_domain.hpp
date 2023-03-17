// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "domain.hpp"

#define DYNAMIX_DEFINE_DOMAIN(tag, ...) \
    ::dynamix::domain& _dynamix_get_domain(tag*) { \
        static ::dynamix::domain the_domain{__VA_ARGS__}; \
        return the_domain; \
    } \
    /* absolutely pointless line, which will require a semicolon at the end of the macro */ \
    struct tag
