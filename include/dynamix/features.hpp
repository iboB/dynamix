// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
* \file
* Non-message feature of mixins.
*/

#include "feature.hpp"

namespace dynamix
{

namespace internal
{
struct mixin_name_feature
{
    const char* name;
};
}

// Allows the mixin name to be set manually (instead of obtained by the class name)
inline internal::mixin_name_feature mixin_name(const char* name)
{
    return{ name };
}

}