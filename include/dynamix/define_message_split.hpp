// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "object.hpp"
#include "internal/mixin_data_in_object.hpp"
#include "internal/message_callers.hpp"
#include "internal/message_macros.hpp"

#include "gen/undef_message_macros.ipp"
#define DYNAMIX_SPLIT_MESSAGE_DEFINE
#include "gen/split_message_macros.ipp"

#include "define_message.hpp"
