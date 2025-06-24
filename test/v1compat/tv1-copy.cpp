// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("v1 obj copying");

using namespace dynamix::v1compat;

class trivial_copy
{
public:
    int i = 0;
};

DYNAMIX_V1_DECLARE_MIXIN(trivial_copy);
DYNAMIX_V1_DEFINE_MIXIN(trivial_copy, none);

class special_copy {
public:
    special_copy() = default;
    special_copy(const special_copy& other)
        : i(other.i + 1)
        , cc(1)
    {}
    special_copy& operator=(const special_copy& other) {
        i = other.i + 2;
        ++a;
        return *this;
    }
    int i = 0;
    int cc = 0;
    int a = 0;
};

DYNAMIX_V1_DECLARE_MIXIN(special_copy);
DYNAMIX_V1_DEFINE_MIXIN(special_copy, none);

class no_copy {
public:
    no_copy() = default;
    no_copy(const no_copy&) = delete;
    no_copy& operator=(const no_copy& other) = delete;
    int i = 0;
};

DYNAMIX_V1_DECLARE_MIXIN(no_copy);
DYNAMIX_V1_DEFINE_MIXIN(no_copy, none);

TEST_CASE("obj_copy") {
    object osrc1;
    mutate(osrc1)
        .add<trivial_copy>()
        .add<special_copy>();

    osrc1.get<trivial_copy>()->i = 2;
    osrc1.get<special_copy>()->i = 5;

    CHECK(osrc1.copyable());

    object c1 = osrc1.copy();
    CHECK(c1.get_type() == osrc1.get_type());
    CHECK(c1.get<trivial_copy>()->i == 2);
    CHECK(c1.get<special_copy>()->i == 6);
    CHECK(c1.get<special_copy>()->cc == 1);
    CHECK(c1.get<special_copy>()->a == 0);

    object osrc2;
    mutate(osrc2)
        .add<special_copy>();

    osrc2.get<special_copy>()->i = 3;

    CHECK(osrc2.copyable());

    c1.copy_matching_from(osrc2);
    CHECK(c1.get_type() == osrc1.get_type());
    CHECK(c1.get<trivial_copy>()->i == 2);
    CHECK(c1.get<special_copy>()->i == 5);
    CHECK(c1.get<special_copy>()->cc == 1);
    CHECK(c1.get<special_copy>()->a == 1);

    object c2;
    c2.copy_matching_from(osrc1);
    CHECK(c2.empty());

    mutate(c2)
        .add<trivial_copy>()
        .add<special_copy>();

    c2.get<trivial_copy>()->i = 10;
    c2.get<special_copy>()->i = 20;
    c2.copy_matching_from(osrc2);
    CHECK(c2.get_type() == c1.get_type());
    CHECK(c2.get<trivial_copy>()->i == 10);
    CHECK(c2.get<special_copy>()->i == 5);
    CHECK(c2.get<special_copy>()->cc == 0);
    CHECK(c2.get<special_copy>()->a == 1);

    c1.copy_from(osrc2);
    CHECK(c1.get_type() == osrc2.get_type());
    CHECK(c1.get<special_copy>()->i == 5);
    CHECK(c1.get<special_copy>()->cc == 1);
    CHECK(c1.get<special_copy>()->a == 2);

    c1.copy_from(osrc1);
    CHECK(c1.get_type() == osrc1.get_type());
    CHECK(c1.get<trivial_copy>()->i == 2);
    CHECK(c1.get<special_copy>()->i == 7);
    CHECK(c1.get<special_copy>()->cc == 1);
    CHECK(c1.get<special_copy>()->a == 3);
}

TEST_CASE("obj_copy_fail") {
    object o1;
    mutate(o1)
        .add<special_copy>()
        .add<trivial_copy>()
        .add<no_copy>();

    CHECK(!o1.copyable());
    // v2!: all exceptions here are object_error
    CHECK_THROWS_AS(object foo = o1.copy(), dynamix::object_error);

    object empty;
    object o2;
    mutate(o2)
        .add<special_copy>()
        .add<trivial_copy>()
        .add<no_copy>();

    CHECK(!o2.copyable());
    CHECK_THROWS_AS(empty.copy_from(o2), dynamix::object_error);

    object o3;
    mutate(o3)
        .add<trivial_copy>()
        .add<no_copy>();

    CHECK(!o3.copyable());
    CHECK_THROWS_AS(empty.copy_from(o3), dynamix::object_error);

    object o4;
    mutate(o4)
        .add<trivial_copy>()
        .add<special_copy>();
    CHECK_NOTHROW(o1.copy_matching_from(o4));
    CHECK_NOTHROW(o4.copy_matching_from(o1));

    CHECK_THROWS_AS(o4.copy_from(o1), dynamix::object_error);
}
