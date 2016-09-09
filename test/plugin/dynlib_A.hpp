// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include <dynamix/dynamix.hpp>

#if defined(DYNLIB_A_SRC)
#   define DYNLIB_A_API DYNAMIX_SYMBOL_EXPORT
#else
#   define DYNLIB_A_API DYNAMIX_SYMBOL_IMPORT
#endif

DYNAMIX_DECLARE_EXPORTED_MIXIN(DYNLIB_A_API, dynlib_a_mixin1);
DYNAMIX_DECLARE_EXPORTED_MIXIN(DYNLIB_A_API, dynlib_a_mixin2);

DYNAMIX_EXPORTED_MESSAGE_0(DYNLIB_A_API, int, dl_a_mixin_specific);
DYNAMIX_EXPORTED_MESSAGE_0(DYNLIB_A_API, int, dl_a_exported);
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(DYNLIB_A_API, int, dl_a_multicast);
