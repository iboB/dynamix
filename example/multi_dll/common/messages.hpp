// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, trace, std::ostream&, o);

DYNAMIX_MESSAGE_1(void, set_id, int, id);
DYNAMIX_CONST_MESSAGE_0(int, get_id);

DYNAMIX_MESSAGE_1(void, add_value, int, value);
DYNAMIX_CONST_MESSAGE_0(int, get_value);

