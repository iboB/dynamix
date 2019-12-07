// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include <dynamix/declare_message.hpp>

DYNAMIX_CONST_MESSAGE_1_OVERLOAD(render_on_target, void, render, int, target);
DYNAMIX_CONST_MESSAGE_0_OVERLOAD(render_on_backbuffer, void, render);

DYNAMIX_MESSAGE_1(void, set_casts_shadows, bool, set);

#define all_rendering_messages render_on_target_msg & render_on_backbuffer_msg & set_casts_shadows_msg

DYNAMIX_MESSAGE_0(void, opengl_specific);
DYNAMIX_MULTICAST_MESSAGE_0(void, opengl_specific_multicast);
