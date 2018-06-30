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

/// The type of the `none` feature.
struct DYNAMIX_API no_features_t {};

/// Use this as a mixin feature to indicate that the mixin
/// implements no features.
extern DYNAMIX_API no_features_t* none;

namespace internal
{

// noop feature used to discard features in the feature list
struct DYNAMIX_API noop_feature_t {};

// noop overloads
inline noop_feature_t* noop_feature(const char*)
{
    return nullptr;
}

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