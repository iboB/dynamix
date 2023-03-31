// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/v1compat/combinators.hpp>
#include <dynamix/v1compat/next_bidder.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("v1 msg default impl");

using namespace dynamix::v1compat;

DYNAMIX_V1_DECLARE_MIXIN(mix_a);
DYNAMIX_V1_DECLARE_MIXIN(mix_b);
DYNAMIX_V1_DECLARE_MIXIN(mix_c);

DYNAMIX_V1_MESSAGE_0(int, basic);
DYNAMIX_V1_MESSAGE_1(int, basic_1, int, x);
DYNAMIX_V1_MESSAGE_1(void, setn, int, n);

// basic msg with default impl
DYNAMIX_V1_MESSAGE_0(int, basic_def);

// basic msg with default impl but not implemented in any mixin
DYNAMIX_V1_MESSAGE_0(int, basic_def_no_impl);

// basic msg with 1 arg and default impl
DYNAMIX_V1_MESSAGE_1(int, basic_1_def, int, x);

// basic const msg with 2 arg and default impl
DYNAMIX_V1_CONST_MESSAGE_2(int, basic_2_def, int, x, int, y);

// overloaded message with default impl
DYNAMIX_V1_MESSAGE_1_OVERLOAD(basic_2_def_overload, int, basic_2_def, const std::string&, str);

// basic multicast test
DYNAMIX_V1_CONST_MULTICAST_MESSAGE_1(int, def_multi, int, i);
DYNAMIX_V1_CREATE_COMBINATOR_CALL_1(int, def_multi, int, i);

////////////////////////////////////////////////////////////////////////////////
// tests
TEST_CASE("basic_msgs")
{
    object o;

    // v2!: empty objects implement messages with default impl
    CHECK(o.implements(basic_def_msg));
    CHECK(!o.get_type().implements_strong<basic_def_msg_t>());
    // v2!: no implements with default

#if DYNAMIX_V1_USE_EXCEPTIONS
    CHECK_THROWS_AS(basic_def(o), bad_message_call);
#endif

    mutate(o)
        .add<mix_a>()
        .add<mix_b>();

    setn(o, 50);

    CHECK(o.implements(basic_def_no_impl_msg));
    CHECK(!o.get_type().implements_strong<basic_def_no_impl_msg_t>());
    CHECK(o.implements(basic_def_msg));
    CHECK(!o.get_type().implements_strong<basic_def_msg_t>());

    CHECK(1110 == basic_def(o));
    CHECK(2111 == basic_def_no_impl(o));
    CHECK(83 == basic_1_def(o, 3));

    mutate(o)
        .add<mix_c>();

    CHECK(1000 == basic_def(o));
    CHECK(o.get_type().implements_strong<basic_def_msg_t>());
    CHECK(2001 == basic_def_no_impl(o));
    CHECK(55 == basic_1_def(o, 5));
}

TEST_CASE("overloads")
{
    object o;

    mutate(o)
        .add<mix_a>()
        .add<mix_b>();

    CHECK(50 == basic_2_def(o, 5, 10));
    CHECK(50 == basic_2_def(o, "50"));

    mutate(o)
        .add<mix_c>();

    CHECK(15 == basic_2_def(o, 5, 10));
    CHECK(2 == basic_2_def(o, "50"));
}

TEST_CASE("multi")
{
    object o;

    mutate(o)
        .add<mix_c>();

    CHECK(o.implements(def_multi_msg));
    CHECK(!o.get_type().implements_strong<def_multi_msg_t>());
    CHECK(400 == def_multi<combinators::sum>(o, 2));

    mutate(o)
        .add<mix_a>()
        .add<mix_b>();

    CHECK(o.get_type().implements_strong<def_multi_msg_t>());
    // v2!: no num_implementers

    setn(o, 3);

    CHECK(70 == def_multi<combinators::sum>(o, 20));
}

////////////////////////////////////////////////////////////////////////////////
// implementations

class mix_a
{
public:
    int basic()
    {
        return 100;
    }

    int def_multi(int n) const
    {
        return n - 10;
    }
};

class mix_b
{
public:
    int basic_1(int x)
    {
        return n + x;
    }

    void setn(int nn)
    {
        n = nn;
    }

    int def_multi(int foo) const
    {
        return n * foo;
    }

    int n;
};

class mix_c
{
public:
    int basic_def()
    {
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(basic_def_no_impl_msg));
        return 1000;
    }

    int basic_1_def(int x)
    {
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(basic_def_no_impl_msg));
        return basic_1(*dm_v1_this, x);
    }

    int basic_2_def(int x, int y) const
    {
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(basic_def_no_impl_msg));
        return x + y;
    }

    int basic_2_def(const std::string& str)
    {
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(basic_def_no_impl_msg));
        return int(str.length());
    }
};

DYNAMIX_V1_DEFINE_MIXIN(mix_a, basic_msg & def_multi_msg);
DYNAMIX_V1_DEFINE_MIXIN(mix_b, basic_1_msg & setn_msg & def_multi_msg);
DYNAMIX_V1_DEFINE_MIXIN(mix_c, basic_def_msg & basic_1_def_msg & basic_2_def_msg & basic_2_def_overload_msg);

DYNAMIX_V1_DEFINE_MESSAGE(basic);
DYNAMIX_V1_DEFINE_MESSAGE(basic_1);
DYNAMIX_V1_DEFINE_MESSAGE(setn);

DYNAMIX_V1_DEFINE_MESSAGE_0_WITH_DEFAULT_IMPL(int, basic_def) {
    // v2!: dm_self arg
    return basic(dm_self) + 1010;
}

DYNAMIX_V1_DEFINE_MESSAGE_0_WITH_DEFAULT_IMPL(int, basic_def_no_impl) {
    return basic_def(dm_self) + 1001;
}

DYNAMIX_V1_DEFINE_MESSAGE_1_WITH_DEFAULT_IMPL(int, basic_1_def, int, x) {
    return 10 * x + basic_1(dm_self, x);
}

DYNAMIX_V1_DEFINE_MESSAGE_2_WITH_DEFAULT_IMPL(int, basic_2_def, int, x, int, y) {
    // v2!: dm_self arg may lead to warnings if unused
    (void)dm_self;
    return x * y;
}

DYNAMIX_V1_DEFINE_MESSAGE_1_WITH_DEFAULT_IMPL(int, basic_2_def_overload, const std::string&, str) {
    (void)dm_self;
    return atoi(str.c_str());
}

DYNAMIX_V1_DEFINE_MESSAGE_1_WITH_DEFAULT_IMPL(int, def_multi, int, i) {
    (void)dm_self;
    return 200 * i;
}
