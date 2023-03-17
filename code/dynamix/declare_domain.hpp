// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/bits/pp.h"

namespace dynamix {
class domain;
}

#define DYNAMIX_DECLARE_EXPORTED_DOMAIN(export, tag) \
    struct tag; \
    export ::dynamix::domain& _dynamix_get_domain(tag*)

#define DYNAMIX_DECLARE_DOMAIN(tag) DYNAMIX_DECLARE_EXPORTED_DOMAIN(I_DNMX_PP_EMPTY(), tag)
