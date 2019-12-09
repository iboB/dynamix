// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

// run the code tests with custom user configuratble definitions

#if defined(DYNAMIX_USE_LEGACY_MESSAGE_MACROS)
#   undef DYNAMIX_USE_LEGACY_MESSAGE_MACROS
#else
#   define DYNAMIX_USE_LEGACY_MESSAGE_MACROS
#endif

#define DYNAMIX_NO_BAD_MULTICASTS
#include "core.cpp"

#if !defined(DYNAMIX_NO_MSG_THROW)
TEST_CASE("nothrow_multicast")
{
    object o;

    int n = 91;
    CHECK_NOTHROW(multi(o, n));
    CHECK(n == 91);

    mutate(o)
        .add<counter>()
        .add<type_checker>();

    CHECK_NOTHROW(multi(o, n));
    CHECK(n == 94);
}
#endif