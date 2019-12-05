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
 * Assertions
 */

#include <cassert>

#define I_DYNAMIX_ASSERT(x) assert(x)
#define I_DYNAMIX_ASSERT_MSG(x, msg) assert((x) && (msg));