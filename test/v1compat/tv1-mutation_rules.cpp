// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/v1compat/mutation_rule.hpp>
#include <dynamix/v1compat/common_mutation_rules.hpp>
#include <iostream>

#include <doctest/doctest.h>

DYNAMIX_V1_DECLARE_MIXIN(a);
DYNAMIX_V1_DECLARE_MIXIN(b);
DYNAMIX_V1_DECLARE_MIXIN(c);

class a {};
class b {};
class c {};

DYNAMIX_V1_DEFINE_MIXIN(a, dynamix::v1compat::none);
DYNAMIX_V1_DEFINE_MIXIN(b, dynamix::v1compat::none);
DYNAMIX_V1_DEFINE_MIXIN(c, dynamix::v1compat::none);

using namespace dynamix::v1compat;

TEST_SUITE_BEGIN("mutation rules");

// v2!: custom rules are different
// simple dependency rule
class custom_rule : public mutation_rule {
public:
    void apply_to(dynamix::type_mutation& mutation) override {
        // v2!: mutation is different: no realtive checks
        if (mutation.has<a>()) {
            mutation.add_if_lacking<b>();
        }
        else {
            mutation.remove<b>();
        }
    }
};

TEST_CASE("custom rule") {
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

TEST_CASE("dependent") {
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

    // v2!: this relation is no longer possible
    // b is alays bound to a
    //CHECK(o.has<a>());
    //CHECK(!o.has<b>());
    //CHECK(!o.has<c>());

    //mutate(o)
    //    .add<b>();

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
    // v2!: this relation is no longer possible:
    // b is always bound to a
    CHECK_FALSE(o.has<b>());
    CHECK(o.has<c>());

    auto r = remove_mutation_rule(id);
    CHECK(r.get() == rule);
}

// v2!: bundles are not possible
//TEST_CASE("bundled")

TEST_CASE("substitute") {
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

TEST_CASE("mutually exclusive") {
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

TEST_CASE("mandatory") {
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

TEST_CASE("deprecated") {
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
