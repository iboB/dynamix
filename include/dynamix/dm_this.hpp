// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "object_of.hpp"

/**
 * \brief a pointer to the owning object of the current mixin
 *
 * Much like `this` is a pointer to the current class, `dm_this`
 * is a macro that, for mixins, points to the current object.
 *
 * It is nothing more than `dynamix::object_of(this)`
 *
 * \note You can disable the definition of this macro by
 * defining `DYNAMIX_NO_DM_THIS` before including the
 * library's headers.
 *
 * \see object_of()
*/
#   define dm_this ::dynamix::object_of(this)
