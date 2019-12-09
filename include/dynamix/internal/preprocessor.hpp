// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
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

#define I_DYNAMIX_PP_EMPTY()

#define I_DYNAMIX_PP_CAT(a, b) I_DYNAMIX_PP_INTERNAL_CAT(a, b)
#define I_DYNAMIX_PP_INTERNAL_CAT(a, b) a##b

#define I_DYNAMIX_PP_STRINGIZE(x) #x

#define I_DYNAMIX_MAYBE_UNUSED(x) (void)(x)