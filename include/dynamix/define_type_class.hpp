// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"
#include "type_class.hpp"
#include "message.hpp"
#include "internal/assert.hpp"

namespace dynamix
{
/// DSL to define a type class.
/// It allows you to set the match function and optionally register it globally
/// Registered type classes have limitations - you cannot create one if you have existing type infos which match it.
/// However checking if an object belongs to a globally registered type class is considerably faster
class DYNAMIX_API define_type_class
{
public:
    define_type_class(type_class::match_func f)
        : func(f)
    {}

    define_type_class& register_globally()
    {
        do_register = true;
        return *this;
    }

    type_class::match_func func;
    bool do_register = false;
};

}
