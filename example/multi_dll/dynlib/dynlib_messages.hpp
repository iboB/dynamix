// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"

DYNAMIX_EXPORTED_MESSAGE_0(DYNLIB_API, void, dl_mixin_specific);
DYNAMIX_EXPORTED_MESSAGE_0(DYNLIB_API, void, dl_exported);

DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(DYNLIB_API, void, dl_multicast);

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(DYNLIB_API, void, trace, std::ostream&, o);

