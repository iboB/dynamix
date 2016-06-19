// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Some useful preprocessor macros used internally
 */

#define _DYNAMIX_PP_EMPTY()

#define _DYNAMIX_PP_CAT(a, b) _DYNAMIX_PP_INTERNAL_CAT(a, b)
#define _DYNAMIX_PP_INTERNAL_CAT(a, b) a##b

#define _DYNAMIX_PP_STRINGIZE(x) #x
