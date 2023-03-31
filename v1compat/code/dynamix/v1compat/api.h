// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if DYNAMIX_V1COMPAT_SHARED
#   if BUILDING_DYNAMIX_V1COMPAT
#       define DYNAMIX_V1COMPAT_API SYMBOL_EXPORT
#   else
#       define DYNAMIX_V1COMPAT_API SYMBOL_IMPORT
#   endif
#else
#   define DYNAMIX_V1COMPAT_API
#endif
