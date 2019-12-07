// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "dynlib_A.hpp"
#include <dynamix/dynamix.hpp>

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

DYNAMIX_CONST_MESSAGE_0(int, plugin_a_local);
DYNAMIX_DEFINE_MESSAGE(plugin_a_local);

class mixin_a
{
public:
    int dl_a_exported()
    {
        return -::dl_a_multicast<combinators::sum>(*dm_this);
    }

    int dl_a_multicast()
    {
        return ::plugin_a_local(dm_this);
    }

    int plugin_a_local() const
    {
        return 102;
    }
};

DYNAMIX_DEFINE_MIXIN(mixin_a, dl_a_exported_msg & dl_a_multicast_msg & plugin_a_local_msg);
