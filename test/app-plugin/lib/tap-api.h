// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if BUILDING_TAP
#   define TAP_API SYMBOL_EXPORT
#else
#   define TAP_API SYMBOL_IMPORT
#endif
