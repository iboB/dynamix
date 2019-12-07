// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include <dynamix/declare_message.hpp>

DYNAMIX_CONST_MESSAGE_0(int, get_combined_transform);

DYNAMIX_MESSAGE_1(void, translate, int, to);
DYNAMIX_MESSAGE_1(void, rotate, int, with);
