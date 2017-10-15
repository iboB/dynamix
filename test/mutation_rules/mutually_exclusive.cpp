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

TEST_CASE("mutually exclusive")
{
    mutually_exclusive_mixins* rule = new mutually_exclusive_mixins;

    rule->add<a>();
    rule->add<b>();

    add_mutation_rule(rule);

    object o;

    mutate(o)
        .add<a>()
        .add<c>();

    CHECK(o.has<a>());
    CHECK(!o.has<b>());
    CHECK(o.has<c>());

    mutate(o).add<b>();
    CHECK(!o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());

    mutate(o).add<a>();
    CHECK(o.has<a>());
    CHECK(!o.has<b>());
    CHECK(o.has<c>());
}
