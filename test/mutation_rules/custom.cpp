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

// simple dependency rule
class custom_rule : public mutation_rule
{
public:
    void apply_to(object_type_mutation& mutation)
    {
        if(mutation.is_adding<a>())
        {
            mutation.start_adding<b>();
        }
        else if(mutation.is_removing<a>())
        {
            mutation.start_removing<b>();
        }
    }
};

TEST_SUITE("mutation rules");

TEST_CASE("mutation rules")
{
    auto id = add_mutation_rule(new custom_rule());

    CHECK(id == 0);

    object o;

    mutate(o)
        .add<a>()
        .add<c>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());

    mutate(o)
        .remove<a>();

    CHECK(!o.has<a>());
    CHECK(!o.has<b>());
    CHECK(o.has<c>());

    auto rule = remove_mutation_rule(id);

    mutate(o)
        .add<a>();

    CHECK(o.has<a>());
    CHECK(!o.has<b>());
    CHECK(o.has<c>());

    mutate(o)
        .add<b>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());

    id = add_mutation_rule(rule);
    CHECK(id == 0);

    auto depr = std::make_shared<deprecated_mixin<c>>();
    id = add_mutation_rule(depr);
    CHECK(id == 1);

    mutate(o)
        .remove<a>();

    CHECK(o.empty());

    CHECK(rule == remove_mutation_rule(0));

    mutate(o)
        .add<a>()
        .add<c>();

    CHECK(o.has<a>());
    CHECK(!o.has<b>());
    CHECK(!o.has<c>());

    rule.reset();
    CHECK(rule == remove_mutation_rule(123));
}
