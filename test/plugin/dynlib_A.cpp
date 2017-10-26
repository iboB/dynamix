// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#define DYNLIB_A_SRC
#include "dynlib_A.hpp"

DYNAMIX_DEFINE_MESSAGE(dl_a_mixin_specific);
DYNAMIX_DEFINE_MESSAGE(dl_a_exported);
DYNAMIX_DEFINE_MESSAGE(dl_a_multicast);

class dynlib_a_mixin1
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "dynlib_a_mixin1"; }
#endif
    int dl_a_mixin_specific()
    {
        return 101;
    }

    int dl_a_multicast()
    {
        return 11;
    }
};

DYNAMIX_DEFINE_MIXIN(dynlib_a_mixin1, dl_a_mixin_specific_msg & dl_a_multicast_msg);


class dynlib_a_mixin2
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "dynlib_a_mixin2"; }
#endif
    int dl_a_mixin_specific()
    {
        return 102;
    }

    int dl_a_multicast()
    {
        return 12;
    }
};

DYNAMIX_DEFINE_MIXIN(dynlib_a_mixin2, priority(1, dl_a_mixin_specific_msg) & dl_a_multicast_msg);
