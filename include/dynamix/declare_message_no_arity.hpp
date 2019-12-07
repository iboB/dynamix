// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#if !defined(DYNAMIX_MESSAGE_0)
static_assert(false, "You must include some arity header before using the no-arity macros");
#endif

// macro which gives out a sensible error if a no-arity macro is called with a bad number of arguments
/// \internal
#define I_DYNAMIX_MESSAGE_ARG_ERROR static_assert(false, "DynaMix macro called with a bad number of arguments")

// a workaround to a visaul c issue which doesn't expand __VA_ARGS__ but inead gives them as a single argument
/// \internal
#define I_DYNAMIX_VA_ARGS_PROXY(MACRO, args) MACRO args

#include "gen/no_arity_message_macros.ipp"
