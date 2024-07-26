// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../bits/pp.h"
#include <splat/pp_select.h>

#define I_DNMX_MSG_STRUCT_NAME(msg_name) I_DNMX_PP_CAT(msg_name, _msg)

#define I_DNMX_DECL_ARG(t, i) t I_DNMX_PP_CAT(a, i)
#define I_DNMX_DECL_ARGS(...) SPLAT_ITERATE_WITH(I_DNMX_DECL_ARG, ##__VA_ARGS__)
