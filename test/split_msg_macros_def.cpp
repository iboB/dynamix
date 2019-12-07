// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#define DYNAMIX_NO_MESSAGE_MACROS
#include <dynamix/dynamix.hpp>

#include <dynamix/gen/undef_message_macros.ipp>
#define DYNAMIX_SPLIT_MESSAGE_DEFINE
#include "split_msg_macros_messages.inl"

DYNAMIX_DEFINE_MESSAGE(dummy);
DYNAMIX_DEFINE_MESSAGE(get_self);
DYNAMIX_DEFINE_MESSAGE(unused);
DYNAMIX_DEFINE_MESSAGE(multi);
DYNAMIX_DEFINE_MESSAGE(inherited);
