// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if TESTLIB_SHARED
#   if BUILDING_TESTLIB
#       define TESTLIB_API SYMBOL_EXPORT
#   else
#       define TESTLIB_API SYMBOL_IMPORT
#   endif
#else
#   define TESTLIB_API
#endif
