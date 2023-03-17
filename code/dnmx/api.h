// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if DYNAMIX_SHARED
#   if BUILDING_DYNAMIX
#       define DYNAMIX_API SYMBOL_EXPORT
#   else
#       define DYNAMIX_API SYMBOL_IMPORT
#   endif
#else
#   define DYNAMIX_API
#endif
