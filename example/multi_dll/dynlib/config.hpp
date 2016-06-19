// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#if defined(DYNLIB_SRC)
#   define DYNLIB_API DYNAMIX_SYMBOL_EXPORT
#else
#   define DYNLIB_API DYNAMIX_SYMBOL_IMPORT
#endif