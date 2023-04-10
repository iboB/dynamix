// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/exception.hpp>
#include <algorithm>
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("by string");

using namespace dynamix::v1compat;

class mixin_a {};
DYNAMIX_V1_DEFINE_MIXIN(mixin_a, none);

struct other_mixin {};
DYNAMIX_V1_DEFINE_MIXIN(other_mixin, none);

class third {};
DYNAMIX_V1_DEFINE_MIXIN(third, mixin_name("3rd") & none);

class unused {};
DYNAMIX_V1_DEFINE_MIXIN(unused, none);

TEST_CASE("v1 mixin_types") {
    auto& di = domain::instance();

    // v2!: no more get_mixin_id_by_name
    auto ainfo = di.get_mixin_info("mixin_a");
    auto ominfo = di.get_mixin_info("other_mixin");
    auto tinfo = di.get_mixin_info("3rd");
    auto uinfo = di.get_mixin_info("unused");

    uint32_t ids[] = {ainfo->id.i, ominfo->id.i, tinfo->id.i, uinfo->id.i};
    std::sort(ids, ids + 4);
    CHECK(ids[0] == 0);
    CHECK(ids[1] == 1);
    CHECK(ids[2] == 2);
    CHECK(ids[3] == 3);

    auto iinfo = di.get_mixin_info("asdfasdf");
    CHECK_FALSE(iinfo);

    iinfo = di.get_mixin_info("MIXIN_A");
    CHECK_FALSE(iinfo);

    object o;

    auto mut = mutate(o);

    CHECK_NOTHROW(mut.add(*ainfo));
    CHECK_NOTHROW(mut.add(*ominfo));
    CHECK_NOTHROW(mut.add(*tinfo));

    CHECK_THROWS_AS(mut.add("third"), dynamix::type_error);

    mut.apply();

    CHECK(o.has<mixin_a>());
    CHECK(o.has<other_mixin>());
    CHECK(o.has<third>());
    CHECK(!o.has<unused>());

    // v2!: no has/get by id
}

TEST_CASE("mixin_names")
{
    auto& dom = domain::instance();
    // v2!: no more type templates
    dynamix::type_mutation mut(dom);

    CHECK(mut.safe_add("mixin_a"));
    CHECK(mut.safe_add("other_mixin"));
    CHECK(mut.safe_add("3rd"));

    // v2!: safe_add to avoid exceptions
    CHECK(!mut.safe_add("asdf"));
    CHECK(!mut.safe_add("mixin_axxx"));
    CHECK(!mut.safe_add("MIXIN_A"));

    auto& type = dom.get_type(std::move(mut));

    object o(type);

    CHECK(o.has<mixin_a>());
    CHECK(o.has<other_mixin>());
    CHECK(o.has<third>());
    CHECK(!o.has<unused>());
    CHECK(o.has("mixin_a"));
    CHECK(o.has("other_mixin"));
    CHECK(o.has("3rd"));
    CHECK(!o.has("unused"));

    // v2!: no single_object_mutator
    auto mutator = mutate(o);

    mutator.remove("unused");
    mutator.remove("other_mixin");
    mutator.remove("3rd");
    mutator.remove("other_mixinxxx");
    mutator.remove("OTHER_MIXIN");

    mutator.apply();

    CHECK(o.has<mixin_a>());
    CHECK(!o.has<other_mixin>());
    CHECK(!o.has<third>());
    CHECK(!o.has<unused>());
    CHECK(o.has("mixin_a"));
    CHECK(!o.has("other_mixin"));
    CHECK(!o.has("3rd"));
    CHECK(!o.has("unused"));
}
