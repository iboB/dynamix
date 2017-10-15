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

TEST_CASE("bundled")
{
    auto bundle = new bundled_mixins;
    bundle->add<a>();
    bundle->add<b>();
    add_mutation_rule(bundle);

    object o;

    mutate(o)
        .add<a>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(!o.has<c>());

    mutate(o)
        .remove<b>();

    CHECK(o.empty());

    mutate(o)
        .add<b>()
        .add<c>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());

    mutate(o)
        .remove<a>();

    CHECK(!o.has<a>());
    CHECK(!o.has<b>());
    CHECK(o.has<c>());
}
