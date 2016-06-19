// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_SUITE("overloads");

using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(has_unio1_multio1);
DYNAMIX_DECLARE_MIXIN(has_unio2_multio2);
DYNAMIX_DECLARE_MIXIN(has_unio3_multio1);
DYNAMIX_DECLARE_MIXIN(has_unio4_multio2);

DYNAMIX_MULTICAST_MESSAGE_1_OVERLOAD(multi_1_overload_1, int, multi, int&, out);
DYNAMIX_MULTICAST_MESSAGE_2_OVERLOAD(multi_1_overload_2, int, multi, int&, out, int, a1);

DYNAMIX_MESSAGE_0_OVERLOAD(uni_1_overload_1, int, uni);
DYNAMIX_MESSAGE_1_OVERLOAD(uni_1_overload_2, int, uni, int, a1);
DYNAMIX_MESSAGE_2_OVERLOAD(uni_1_overload_3, int, uni, int, a1, int, a2);
DYNAMIX_MESSAGE_3_OVERLOAD(uni_1_overload_4, int, uni, int, a1, int, a2, int, a3);

TEST_CASE("overloads")
{
    object o;
    mutate(o)
        .add<has_unio1_multio1>()
        .add<has_unio2_multio2>()
        .add<has_unio3_multio1>()
        .add<has_unio4_multio2>();

    int a1 = 1, a2 = 2, a3 = 3;

    CHECK(uni(o) == 0);
    CHECK(uni(o, a1) == 1);
    CHECK(uni(o, a1, a2) == 3);
    CHECK(uni(o, a1, a2, a3) == 6);

    int out = 0;

    multi(o, out);
    CHECK(out == 1); // 0 + 1
    out = 0;

    multi(o, out, a1);
    CHECK(out == 3); // a1 + a1 + 1
    out = 0;

    combinators::sum<int> s;

    multi(o, out, s);
    CHECK(s.result() == 1); // 0 + 0 + 1
    s.reset();
    out = 0;

    multi(o, out, a1, s);
    CHECK(s.result() == 4); // a1 + (a1 + a1 + 1)
}

class has_unio1_multio1
{
public:
    int uni() { return 0; }
    int multi(int& out) { return out += 0; }
};

class has_unio2_multio2
{
public:
    int uni(int a) { return a; }
    int multi(int& out, int a) { return out += a; }
};

class has_unio3_multio1
{
public:
    int uni(int a1, int a2) { return a1 + a2; }
    int multi(int& out) { return out += 1; }
};

class has_unio4_multio2
{
public:
    int uni(int a1, int a2, int a3) { return a1 + a2 + a3; }
    int multi(int& out, int a) { return out += a + 1; }
};

// this order should be important if the messages aren't sorted by mixin name
DYNAMIX_DEFINE_MIXIN(has_unio1_multio1, uni_1_overload_1_msg & multi_1_overload_1_msg);
DYNAMIX_DEFINE_MIXIN(has_unio2_multio2, uni_1_overload_2_msg & multi_1_overload_2_msg);
DYNAMIX_DEFINE_MIXIN(has_unio3_multio1, uni_1_overload_3_msg & multi_1_overload_1_msg);
DYNAMIX_DEFINE_MIXIN(has_unio4_multio2, uni_1_overload_4_msg & multi_1_overload_2_msg);

DYNAMIX_DEFINE_MESSAGE(multi_1_overload_1);
DYNAMIX_DEFINE_MESSAGE(multi_1_overload_2);
DYNAMIX_DEFINE_MESSAGE(uni_1_overload_1);
DYNAMIX_DEFINE_MESSAGE(uni_1_overload_2);
DYNAMIX_DEFINE_MESSAGE(uni_1_overload_3);
DYNAMIX_DEFINE_MESSAGE(uni_1_overload_4);
