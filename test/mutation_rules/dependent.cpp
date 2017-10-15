// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "test_mixins.hpp"

using namespace dynamix;

TEST_SUITE("mutation rules");

TEST_CASE("dependent")
{
    auto rule = new dependent_mixins;
    rule->set_master<a>();
    rule->add<b>();
    add_mutation_rule(rule);

    object o;

    mutate(o)
        .add<a>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(!o.has<c>());

    mutate(o)
        .remove<b>();

    CHECK(o.has<a>());
    CHECK(!o.has<b>());
    CHECK(!o.has<c>());

    mutate(o)
        .add<b>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(!o.has<c>());

    mutate(o)
        .remove<a>();

    CHECK(o.empty());

    mutate(o)
        .add<b>()
        .add<c>();

    CHECK(!o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());
}
