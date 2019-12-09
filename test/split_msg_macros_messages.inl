// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

DYNAMIX_MESSAGE_0(void, dummy);
DYNAMIX_CONST_MESSAGE_0(const void*, get_self);
DYNAMIX_MESSAGE_0(void, unused);
DYNAMIX_MULTICAST_MESSAGE_1(void, multi, int&, n)
DYNAMIX_MESSAGE_0(int, inherited);
