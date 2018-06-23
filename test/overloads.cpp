// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include "doctest/doctest.h"

TEST_SUITE("overloads");

using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(has_unio1_multio1);
DYNAMIX_DECLARE_MIXIN(has_unio2_multio2);
DYNAMIX_DECLARE_MIXIN(has_unio3_multio1);
DYNAMIX_DECLARE_MIXIN(has_unio4_multio1_multio2);
DYNAMIX_DECLARE_MIXIN(has_unio1_c_unio2);

DYNAMIX_MULTICAST_MESSAGE_1_OVERLOAD(multioverload_1, int, multi, int&, out);
DYNAMIX_MULTICAST_MESSAGE_2_OVERLOAD(multioverload_2, int, multi, int&, out, int, a1);

DYNAMIX_MESSAGE_0_OVERLOAD(unioverload_1, int, uni);
DYNAMIX_CONST_MESSAGE_0_OVERLOAD(unioverload_1c, int, uni);
DYNAMIX_MESSAGE_1_OVERLOAD(unioverload_2, int, uni, int, a1);
DYNAMIX_MESSAGE_2_OVERLOAD(unioverload_3, int, uni, int, a1, int, a2);
DYNAMIX_MESSAGE_3_OVERLOAD(unioverload_4, int, uni, int, a1, int, a2, int, a3);

TEST_CASE("overloads")
{
    object o;
    mutate(o)
        .add<has_unio1_multio1>()
        .add<has_unio2_multio2>()
        .add<has_unio3_multio1>()
        .add<has_unio4_multio1_multio2>();

    int a1 = 1, a2 = 2, a3 = 3;

    CHECK(uni(o) == 0);
    CHECK(uni(o, a1) == 1);
    CHECK(uni(o, a1, a2) == 3);
    CHECK(uni(o, a1, a2, a3) == 6);

    int out = 0;

    multi(o, out);
    CHECK(out == 2); // 0 + 1 + 1
    out = 0;

    multi(o, out, a1);
    CHECK(out == 3); // a1 + a1 + 1
    out = 0;

    combinators::sum<int> s;

    multi(o, out, s);
    CHECK(s.result() == 3); // 0 + 0 + 1 + ((0 + 0 + 1) + 1)
    s.reset();
    out = 0;

    multi(o, out, a1, s);
    CHECK(s.result() == 4); // a1 + (a1 + a1 + 1)

    mutate(o)
        .remove<has_unio1_multio1>()
        .remove<has_unio2_multio2>()
        .add<has_unio1_c_unio2>();

    CHECK(uni(o) == 55);
    CHECK(uni(o, a1) == 81);

    const auto& co = o;
    CHECK(uni(co) == 33);
}

class has_unio1_multio1
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "has_unio1_multio1"; }
#endif

    int uni() { return 0; }
    int multi(int& out) { return out += 0; }
};

class has_unio2_multio2
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "has_unio2_multio2"; }
#endif

    int uni(int a) { return a; }
    int multi(int& out, int a) { return out += a; }
};

class has_unio3_multio1
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "has_unio3_multio1"; }
#endif

    int uni(int a1, int a2) { return a1 + a2; }
    int multi(int& out) { return out += 1; }
};

// test for issue #20
class parent
{
public:
    int multi(int& out, int a) { return out += a + 1; }
};

class has_unio4_multio1_multio2 : public parent
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "has_unio4_multio1_multio2"; }
#endif

    int uni(int a1, int a2, int a3) { return a1 + a2 + a3; }
    int multi(int& out) { return out += 1; }
};

class has_unio1_c_unio2
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "has_unio1_c_unio2"; }
#endif

    int uni() { return 55; }
    int uni() const { return 33; }
    int uni(int a1) { return a1 + 80; }
};

DYNAMIX_DEFINE_MIXIN(has_unio1_multio1, unioverload_1_msg & multioverload_1_msg);
DYNAMIX_DEFINE_MIXIN(has_unio2_multio2, unioverload_2_msg & multioverload_2_msg);
DYNAMIX_DEFINE_MIXIN(has_unio4_multio1_multio2, unioverload_4_msg & multioverload_1_msg & from_parent<parent>(multioverload_2_msg));
DYNAMIX_DEFINE_MIXIN(has_unio1_c_unio2, unioverload_1_msg & unioverload_1c_msg & unioverload_2_msg);
DYNAMIX_DEFINE_MIXIN(has_unio3_multio1, unioverload_3_msg & multioverload_1_msg);

DYNAMIX_DEFINE_MESSAGE(multioverload_1);
DYNAMIX_DEFINE_MESSAGE(multioverload_2);
DYNAMIX_DEFINE_MESSAGE(unioverload_1);
DYNAMIX_DEFINE_MESSAGE(unioverload_1c);
DYNAMIX_DEFINE_MESSAGE(unioverload_2);
DYNAMIX_DEFINE_MESSAGE(unioverload_3);
DYNAMIX_DEFINE_MESSAGE(unioverload_4);
