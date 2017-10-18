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

TEST_CASE("substitute")
{
    add_mutation_rule(new substitute_mixin<a, c>());

    object o;

    mutate(o)
        .add<a>()
        .add<b>();

    CHECK(!o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());
}
