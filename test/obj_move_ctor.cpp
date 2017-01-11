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

TEST_SUITE("obj move ctor");

using namespace dynamix;

class a
{
public:
    a() : val(10) {}
    int val;

    int value() const  { return val; }

    object* get_obj_a() { return dm_this; }
};

class b
{
public:
    b() : val(1) {}
    int val;

    int value() const { return val; }

    const object* get_obj_b() const { return dm_this; }
};

DYNAMIX_CONST_MULTICAST_MESSAGE_0(int, value);
DYNAMIX_MESSAGE_0(object*, get_obj_a);
DYNAMIX_CONST_MESSAGE_0(const object*, get_obj_b);

DYNAMIX_DEFINE_MIXIN(a, value_msg & get_obj_a_msg);
DYNAMIX_DEFINE_MIXIN(b, value_msg & get_obj_b_msg);

DYNAMIX_DEFINE_MESSAGE(value);
DYNAMIX_DEFINE_MESSAGE(get_obj_a);
DYNAMIX_DEFINE_MESSAGE(get_obj_b);

object gen_obj()
{
    object obj;
    mutate(obj)
        .add<a>()
        .add<b>();

    return obj;
}

TEST_CASE("obj_move_ctor")
{
    object empty;

    object o1 = std::move(empty); // to check that this won't crash
    mutate(o1)
        .add<a>()
        .add<b>();

    // sanity
    CHECK(&o1 == get_obj_a(o1));
    CHECK(&o1 == get_obj_b(o1));
    CHECK(11 == value<combinators::sum>(o1));

    // actual tests
    object o1_moved = std::move(o1);

    CHECK(&o1_moved == get_obj_a(o1_moved));
    CHECK(&o1_moved == get_obj_b(o1_moved));
    CHECK(11 == value<combinators::sum>(o1_moved));

    CHECK(!o1.has<a>());
    CHECK(!o1.has<b>());
    CHECK(!o1.implements(value_msg));
    CHECK(!o1.implements(get_obj_a_msg));
    CHECK(!o1.implements(get_obj_b_msg));

    object o2 = gen_obj();

    CHECK(&o2 == get_obj_a(o2));
    CHECK(&o2 == get_obj_b(o2));
    CHECK(11 == value<combinators::sum>(o2));

    std::vector<object> v;

    v.push_back(std::move(o1_moved));
    assert(v.capacity() == 1);

    v.push_back(std::move(o2));

    CHECK(&v.front() == get_obj_a(v.front()));
    CHECK(&v.front() == get_obj_b(v.front()));
    CHECK(11 == value<combinators::sum>(v.front()));

    CHECK(&v.back() == get_obj_a(v.back()));
    CHECK(&v.back() == get_obj_b(v.back()));
    CHECK(11 == value<combinators::sum>(v.back()));
}

TEST_CASE("obj_move_assignment")
{
    object empty;

    object o1;
    o1 = std::move(empty); // to check that this won't crash

    mutate(o1)
        .add<a>()
        .add<b>();

    // sanity
    CHECK(&o1 == get_obj_a(o1));
    CHECK(&o1 == get_obj_b(o1));
    CHECK(11 == value<combinators::sum>(o1));

    // actual tests
    object o1_moved;
    o1_moved = std::move(o1);

    CHECK(&o1_moved == get_obj_a(o1_moved));
    CHECK(&o1_moved == get_obj_b(o1_moved));
    CHECK(11 == value<combinators::sum>(o1_moved));

    CHECK(!o1.has<a>());
    CHECK(!o1.has<b>());
    CHECK(!o1.implements(value_msg));
    CHECK(!o1.implements(get_obj_a_msg));
    CHECK(!o1.implements(get_obj_b_msg));

    object o2;
    o2 = gen_obj();

    CHECK(&o2 == get_obj_a(o2));
    CHECK(&o2 == get_obj_b(o2));
    CHECK(11 == value<combinators::sum>(o2));
}
