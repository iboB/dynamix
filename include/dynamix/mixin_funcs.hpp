// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"

namespace dynamix
{
typedef void(*mixin_constructor_proc)(void* memory);
typedef void(*mixin_copy_proc)(void* memory, const void* source);
typedef void(*mixin_move_proc)(void* memory, void* source);
typedef void(*mixin_destructor_proc)(void* memory);
}
