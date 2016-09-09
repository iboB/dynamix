// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include "dynlib_A.hpp"

using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(mixin_a);

extern "C" DYNAMIX_SYMBOL_EXPORT void modify_object(object* o)
{
    mutate(o).add<mixin_a>();
}

extern "C" DYNAMIX_SYMBOL_EXPORT void release_object(object* o)
{
    mutate(o).remove<mixin_a>();
}

class mixin_a
{
public:
    int dl_a_exported()
    {
        return ::dl_a_multicast<combinators::sum>(*dm_this);
    }

    int dl_a_multicast()
    {
        return 101;
    }
};

DYNAMIX_DEFINE_MIXIN(mixin_a, dl_a_exported_msg & dl_a_multicast_msg);
