// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint> // minimal header that will give us _LIBCPP_VERSION

#define DYNAMIX_FORCE_PMR_COMPAT 0

#if !DYNAMIX_FORCE_PMR_COMPAT || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 16000)
#   define DYNAMIX_HAS_PMR 1
#else
#   define DYNAMIX_HAS_PMR 0
#endif
