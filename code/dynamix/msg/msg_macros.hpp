// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../../dnmx/msg/msg_macros.h"

#define I_DYNAMIX_BASIC_FUNC_TRAITS(func_name) I_DNMX_PP_CAT(_dynamix_basic_traits_for_, func_name)
#define DYNAMIX_FUNC_TRAITS_NAME(func_name) I_DNMX_PP_CAT(_dynamix_traits_for_, func_name)

#define I_DNMX_FWD_ARG(t, i) std::forward<t>(I_DNMX_PP_CAT(a, i))
#define I_DNMX_FWD_ARGS(...) SPLAT_ITERATE_WITH(I_DNMX_FWD_ARG, ##__VA_ARGS__)
