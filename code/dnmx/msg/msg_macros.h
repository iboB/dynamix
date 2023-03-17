// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../bits/pp.h"
#include <splat/pp_select.h>

#define I_DNMX_MSG_STRUCT_NAME(msg_name) I_DNMX_PP_CAT(msg_name, _msg)

#define I_DNMX_ITERATE_0(MACRO)
#define I_DNMX_ITERATE_1(MACRO, t0) MACRO(t0, 0)
#define I_DNMX_ITERATE_2(MACRO, t0, t1) MACRO(t0, 0), MACRO(t1, 1)
#define I_DNMX_ITERATE_3(MACRO, t0, t1, t2) MACRO(t0, 0), MACRO(t1, 1), MACRO(t2, 2)
#define I_DNMX_ITERATE_4(MACRO, t0, t1, t2, t3) MACRO(t0, 0), MACRO(t1, 1), MACRO(t2, 2), MACRO(t3, 3)

#define I_DNMX_ITERATE_BY(MACRO, ...) SPLAT_SELECT_ARITY_UPTO_5(_, I_DNMX_ITERATE_0, I_DNMX_ITERATE_1, I_DNMX_ITERATE_2, I_DNMX_ITERATE_3, I_DNMX_ITERATE_4, MACRO, ##__VA_ARGS__)

#define I_DNMX_DECL_ARG(t, i) t I_DNMX_PP_CAT(a, i)
#define I_DNMX_DECL_ARGS(...) I_DNMX_ITERATE_BY(I_DNMX_DECL_ARG, ##__VA_ARGS__)
