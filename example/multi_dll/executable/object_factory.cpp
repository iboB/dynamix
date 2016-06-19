// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "executable_pch.hpp"
#include "object_factory.hpp"
#include "exe_mixin_fwd.hpp"
#include "../common/mixin_fwd.hpp"
#include "../dynlib/dynlib_mixin_fwd.hpp"

using namespace dynamix;

object_factory::object_factory()
{
    the_object = new object;

    mutate(the_object)
        .add<common_mixin>()
        .add<exe_mixin>()
        .add<dynlib_mixin>()
        ;
}

object_factory::~object_factory()
{
    delete the_object;
}
