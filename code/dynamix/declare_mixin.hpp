// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#include "../dnmx/bits/pp.h"

namespace dynamix {
class common_mixin_info;
}

#define DYNAMIX_DECLARE_EXPORTED_MIXIN(export, mixin) \
    extern export const ::dynamix::common_mixin_info& _dynamix_get_mixin_info(mixin* m)

#define DYNAMIX_DECLARE_MIXIN(mixin) \
    DYNAMIX_DECLARE_EXPORTED_MIXIN(I_DNMX_PP_EMPTY(), mixin)
