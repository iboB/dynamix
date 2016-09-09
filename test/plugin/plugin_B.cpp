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

DYNAMIX_DECLARE_MIXIN(mixin_b);

extern "C" DYNAMIX_SYMBOL_EXPORT void modify_object(object* o)
{
    mutate(o).add<mixin_b>();
}

extern "C" DYNAMIX_SYMBOL_EXPORT void release_object(object* o)
{
    mutate(o).remove<mixin_b>();
}

DYNAMIX_MULTICAST_MESSAGE_0(int, shared);
DYNAMIX_DEFINE_MESSAGE(shared);

class mixin_b
{
public:
    int shared()
    {
        return 7;
    }

    int dl_a_multicast()
    {
        return 1001;
    }
};

DYNAMIX_DEFINE_MIXIN(mixin_b, shared_msg & dl_a_multicast_msg);
