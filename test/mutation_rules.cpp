// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <iostream>

#include "doctest/doctest.h"

#include "test_mixins.hpp"

using namespace dynamix;

TEST_SUITE("mutation rules");

// simple dependency rule
class custom_rule : public mutation_rule
{
public:
    void apply_to(object_type_mutation& mutation)
    {
        if (mutation.is_adding<a>())
        {
            mutation.start_adding<b>();
        }
        else if (mutation.is_removing<a>())
        {
            mutation.start_removing<b>();
        }
    }
};

TEST_SUITE("mutation rules");

TEST_CASE("custom rule")
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

    CHECK(depr == remove_mutation_rule(1));
}

TEST_CASE("dependent")
{
    auto rule = new dependent_mixins;
    rule->set_master<a>();
    rule->add<b>();
    auto id = add_mutation_rule(rule);

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

    auto r = remove_mutation_rule(id);
    CHECK(r.get() == rule);
}

TEST_CASE("bundled")
{
    auto bundle = new bundled_mixins;
    bundle->add<a>();
    bundle->add<b>();
    auto id = add_mutation_rule(bundle);
    CHECK(id == 0);

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

    remove_mutation_rule(id);
}

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

    remove_mutation_rule(0);
}

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

    remove_mutation_rule(0);
}

TEST_CASE("mandatory")
{
    auto id = add_mutation_rule(new mandatory_mixin<c>());
    CHECK(id == 0);

    object o;

    mutate(o)
        .add<a>()
        .add<b>();

    CHECK(o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());

    remove_mutation_rule(0);
}

TEST_CASE("deprecated")
{
    add_mutation_rule(new deprecated_mixin<a>());

    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>();

    CHECK(!o.has<a>());
    CHECK(o.has<b>());
    CHECK(o.has<c>());
}
