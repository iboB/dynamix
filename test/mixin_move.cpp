// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include "doctest/doctest.h"

#include <string>
#include <iostream> // needed on macs for doctest

TEST_SUITE("mixin moving");

using namespace dynamix;

class default_mv
{
public:
    // relying on default generated ops
    std::string data;
    object* get_obj_default() { return dm_this; }
};

class custom_mv
{
public:
    custom_mv() = default;
    custom_mv(const custom_mv&) = delete;
    custom_mv& operator=(const custom_mv&) = delete;
    ~custom_mv() = default;

    custom_mv(custom_mv&& other) = delete;

    custom_mv& operator=(custom_mv&& other)
    {
        n = other.n;
        other.n = -1;
        data = std::move(other.data);
        other.data = "x";
        return *this;
    }

    object* get_obj_custom() { return dm_this; }

    int n = 0;
    std::string data;
};

class nomove
{
public:
    nomove() = default;
    nomove(const nomove&) = delete;
    nomove& operator=(const nomove&) = delete;
    nomove(nomove&& other) = delete;
    nomove& operator=(nomove&& other) = delete;
    ~nomove() = default;
};

DYNAMIX_MESSAGE_0(object*, get_obj_default);

DYNAMIX_DEFINE_MIXIN(default_mv, get_obj_default_msg);

DYNAMIX_DEFINE_MESSAGE(get_obj_default);

DYNAMIX_MESSAGE_0(object*, get_obj_custom);

DYNAMIX_DEFINE_MIXIN(custom_mv, get_obj_custom_msg);

DYNAMIX_DEFINE_MESSAGE(get_obj_custom);

DYNAMIX_DEFINE_MIXIN(nomove, dynamix::none);

TEST_CASE("sanity")
{
    object src;
    mutate(src)
        .add<default_mv>()
        .add<custom_mv>();

    CHECK(get_obj_default(src) == &src);
    CHECK(get_obj_custom(src) == &src);

    object target;
    mutate(target)
        .add<default_mv>()
        .add<custom_mv>();

    CHECK(get_obj_default(target) == &target);
    CHECK(get_obj_custom(target) == &target);

    auto& sd = *src.get<default_mv>();
    auto& sc = *src.get<custom_mv>();

    sd.data = "asdf";
    sc.n = 53;
    sc.data = "qwer";

    auto& td = *target.get<default_mv>();
    auto& tc = *target.get<custom_mv>();

    *target.get<default_mv>() = std::move(*src.get<default_mv>());
    *target.get<custom_mv>() = std::move(*src.get<custom_mv>());

    CHECK(::dynamix::object_of(&sd) == &src);
    CHECK(::dynamix::object_of(&sc) == &src);
    CHECK(::dynamix::object_of(&td) == &target);
    CHECK(::dynamix::object_of(&tc) == &target);

    CHECK(sd.data.empty());
    CHECK(sc.n == -1);
    CHECK(sc.data == "x");

    CHECK(td.data == "asdf");
    CHECK(tc.n == 53);
    CHECK(tc.data == "qwer");
}

TEST_CASE("mixin moves")
{
    object src;
    mutate(src)
        .add<default_mv>()
        .add<custom_mv>();

    object srcd;
    mutate(srcd)
        .add<default_mv>();

    object srcc;
    mutate(srcc)
        .add<custom_mv>();

    object nope;
    mutate(nope)
        .add<nomove>();

    object target;
    mutate(target)
        .add<default_mv>()
        .add<custom_mv>()
        .add<nomove>();

    {
        auto sd = src.get<default_mv>();
        auto sc = src.get<custom_mv>();

        sd->data = "asdf";
        sc->n = 53;
        sc->data = "qwer";

        sd = srcd.get<default_mv>();
        sd->data = "zxcv";

        sc = srcc.get<custom_mv>();
        sc->n = 90;
        sc->data = "yuiop";
    }

    target.move_matching_from(src);

    {
        auto& sd = *src.get<default_mv>();
        auto& sc = *src.get<custom_mv>();
        auto& td = *target.get<default_mv>();
        auto& tc = *target.get<custom_mv>();

        CHECK(sd.data.empty());
        CHECK(sc.n == -1);
        CHECK(sc.data == "x");

        CHECK(td.data == "asdf");
        CHECK(tc.n == 53);
        CHECK(tc.data == "qwer");
    }

    target.move_matching_from(srcd);

    {
        auto& sd = *src.get<default_mv>();
        auto& td = *target.get<default_mv>();
        auto& tc = *target.get<custom_mv>();

        CHECK(sd.data.empty());

        CHECK(td.data == "zxcv");
        CHECK(tc.n == 53);
        CHECK(tc.data == "qwer");
    }

    target.move_matching_from(srcc);

    {
        auto& sc = *src.get<custom_mv>();
        auto& td = *target.get<default_mv>();
        auto& tc = *target.get<custom_mv>();

        CHECK(sc.n == -1);
        CHECK(sc.data == "x");

        CHECK(td.data == "zxcv");
        CHECK(tc.n == 90);
        CHECK(tc.data == "yuiop");
    }

#if DYNAMIX_USE_EXCEPTIONS
    CHECK_THROWS_AS(target.move_matching_from(nope), bad_move_assignment);
#endif

    CHECK(get_obj_default(src) == &src);
    CHECK(get_obj_custom(src) == &src);
    CHECK(get_obj_default(srcd) == &srcd);
    CHECK(get_obj_custom(srcc) == &srcc);

    CHECK(get_obj_default(target) == &target);
    CHECK(get_obj_custom(target) == &target);
}
