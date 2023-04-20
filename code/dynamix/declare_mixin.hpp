// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#include "../dnmx/bits/pp.h"
#include "mixin_info_fwd.hpp"

#define DYNAMIX_DECLARE_EXPORTED_MIXIN(export, mixin) \
    extern export const ::dynamix::mixin_info& _dynamix_get_mixin_info(const mixin*)

#define DYNAMIX_DECLARE_MIXIN(mixin) \
    DYNAMIX_DECLARE_EXPORTED_MIXIN(I_DNMX_PP_EMPTY(), mixin)
