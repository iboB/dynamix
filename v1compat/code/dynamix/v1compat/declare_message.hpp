// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/object.hpp>

#define I_DYNAMIX_V1_MSG_STRUCT_NAME(msg_name) I_DNMX_PP_CAT(msg_name, _msg_t)
#define I_DYNAMIX_V1_MESSAGE_TAG(msg_name) I_DNMX_PP_CAT(msg_name, _msg)
#define I_DYNAMIX_V1_DEFAULT_IMPL_FUNC_NAME(msg_name) I_DNMX_PP_CAT(msg_name, _msg_default_impl)

#include "gen/template_message_macros.ipp"
#include "gen/arity_message_macros.ipp"
#include "gen/create_combinator_call.ipp"
