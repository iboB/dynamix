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
 * Macros used to declare and define messages.
 */

#include "preprocessor.hpp"

/// \internal
#define I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name) I_DYNAMIX_PP_CAT(dynamix_msg_, message_name)
/// \internal
#define I_DYNAMIX_MESSAGE_TAG(message_name) I_DYNAMIX_PP_CAT(message_name, _msg)
/// \internal
#define I_DYNAMIX_MESSAGE_CALLER_STRUCT(mechanism) I_DYNAMIX_PP_CAT(msg_, mechanism)

// default impl helper macros

// name of default implementation struct
/// \internal
#define DYNAMIX_DEFAULT_IMPL_STRUCT(message_name) I_DYNAMIX_PP_CAT(message_name, _default_impl_t)
