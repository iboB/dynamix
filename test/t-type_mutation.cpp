// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <doctest/doctest.h>

#include <dynamix/type_mutation.hpp>
#include <dynamix/type.hpp>
#include <dynamix/exception.hpp>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("type_mutation from empty") {
    test_data t;
    domain dom("e");

    type_mutation mut(dom);
    CHECK(&dom == &mut.dom);
    CHECK(mut.mixins.empty());
    CHECK(mut.implements(*t.serialize)); // default
    CHECK_FALSE(mut.implements_strong(*t.serialize));

    CHECK_THROWS_WITH_AS(mut.add("actor"),
        "e: creating type {}: unknown mixin 'actor' in add",
        type_error);
    CHECK_THROWS_WITH_AS(mut.to_back("actor"),
        "e: creating type {}: unknown mixin 'actor' in to_back",
        type_error);

    t.register_all_mixins(dom);

    mut.add(t.mesh->name.to_std());
    mut.add(*t.ai);
    mut.add("actor");

    CHECK(mut.has(*t.ai));
    CHECK_FALSE(mut.has(*t.empty));
    CHECK(mut.lacks(*t.empty));
    CHECK(mut.implements_strong(*t.serialize));
    CHECK(mut.implements_strong("render"));

    // normalization
    mut.add("mesh");
    mut.dedup();
    CHECK(mut.mixins.size() == 3);
    CHECK(mut.mixins.back() == t.mesh);

    mut.add(*t.mesh);
    CHECK(mut.mixins.size() == 4);
    mut.dedup();
    CHECK(mut.mixins.size() == 3);
}

TEST_CASE("type_mutation from type") {
    test_data t;
    domain dom("t");
    t.register_all_mixins(dom);
    t.create_types(dom);

    {
        type_mutation mut(*t.t_asim);
        CHECK(mut.implements_strong(*t.render));
        CHECK_FALSE(mut.implements_strong(*t.get_name));
        CHECK_FALSE(mut.add_if_lacking(*t.mesh));
        mut.remove("mesh");
        CHECK(mut.lacks(*t.mesh));
        CHECK_FALSE(mut.lacks(*t.stats));
        mut.remove(*t.stats);
        CHECK(mut.lacks(*t.stats));
        CHECK_THROWS_WITH_AS(mut.to_back(*t.stats),
            "t: creating type {'ai', 'immaterial'}: 'stats' to_back on missing mixin",
            type_error);
        CHECK_FALSE(mut.has("actor"));
        CHECK(mut.add_if_lacking(*t.actor));
        CHECK(mut.has("actor"));
        CHECK(mut.implements_strong(*t.get_name));
        CHECK_FALSE(mut.implements_strong(*t.render));
    }
}

TEST_CASE("type_mutation misc") {
    test_data t;
    domain dom;

    t.register_all_mixins(dom);

    {
        type_mutation mut(dom);
        mut.add(*t.actor);
        mut.add(*t.mesh);
        mut.to_back(*t.actor);
        auto& ma = dom.get_type(std::move(mut));

        CHECK(ma.has(*t.mesh));
        CHECK(ma.has(t.actor->id));

        CHECK_FALSE(ma.default_constructible());
        CHECK(ma.copyable());
    }

    {
        type_mutation mut(dom);
        mut.add(*t.movable);
        auto& tmov = dom.get_type(std::move(mut));
        t.create_types(dom);
        CHECK(t.t_mov == &tmov);
    }

    {
        type_mutation mut(*t.t_afmi);
        mut.add(*t.actor);
        mut.add(*t.stats);

        mut.remove(*t.ai);
        CHECK(mut.has("mesh"));
        mut.to_back(*t.mesh);

        mut.remove("flyer");

        mut.add(*t.procedural_geometry);
        mut.to_back("invisible");

        auto& tasmpi = dom.get_type(std::move(mut));

        t.create_more_types(dom);
        CHECK(t.t_asmpi == &tasmpi);
    }

    {
        type_mutation mut(*t.t_asmpi);

        mut.mixins = {t.invisible, t.mesh, t.procedural_geometry, t.stats, t.actor};

        auto& timpsa = dom.get_type(std::move(mut));
        t.create_reordered_types(dom);
        CHECK(t.t_impsa == &timpsa);
    }
}

TEST_CASE("type_mutation canon") {
    test_data t;
    domain_settings s = {};
    s.canonicalize_types = true;
    domain dom({}, s);

    t.register_all_mixins(dom);
    t.create_types(dom);

    {
        type_mutation mut(*t.t_asim);
        mut.add(*t.ai);
        mut.dedup();
        CHECK(&dom.get_type(mut.mixins) == t.t_asim);

        mut.remove(*t.stats);
        mut.remove(*t.ai);
        mut.add(*t.actor);
        mut.remove(*t.actor);
        mut.remove(*t.immaterial);
        CHECK_FALSE(mut.has(*t.actor));
        CHECK(mut.lacks(*t.stats));
        CHECK_FALSE(mut.lacks(*t.mesh));

        CHECK(&dom.get_type(std::move(mut)) == t.t_m);
    }

    {
        type_mutation mut(dom);
        mut.add(*t.mesh);
        mut.add(*t.stats);
        mut.add(*t.immaterial);
        mut.add(*t.ai);
        CHECK(&dom.get_type(mut.mixins) == t.t_asim);

        mut.remove(*t.stats);
        mut.remove(*t.ai);
        mut.add(*t.actor);
        mut.remove(*t.actor);
        mut.remove(*t.immaterial);
        CHECK(mut.lacks(*t.actor));
        CHECK(mut.lacks(*t.immaterial));
        CHECK(mut.has("mesh"));
        CHECK(&dom.get_type(mut.mixins) == t.t_m);

        mut.add(*t.ai);
        mut.add(*t.invisible);
        mut.add(*t.flyer);

        CHECK(mut.has("mesh"));
        CHECK(mut.has("flyer"));
        CHECK(&dom.get_type(std::move(mut)) == t.t_afmi);
    }

    {
        t.create_more_types(dom);
        type_mutation mut(*t.t_asmpi);

        mut.mixins = {t.invisible, t.mesh, t.procedural_geometry, t.stats, t.actor};

        auto& tasmpi = dom.get_type(std::move(mut));
        CHECK(t.t_asmpi == &tasmpi);
    }
}
