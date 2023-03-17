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
    domain dom;

    type_mutation mut(dom);
    CHECK(&dom == &mut.get_domain());
    CHECK(&dom.get_empty_type() == &mut.base());
    CHECK(mut.noop());
    CHECK_FALSE(mut.adding_mixins());
    CHECK_FALSE(mut.removing_mixins());
    auto& nt = mut.new_type();
    CHECK(nt.mixins.empty());
    CHECK(nt.implements(*t.serialize)); // default
    CHECK_FALSE(nt.implements_strong(*t.serialize));

    CHECK_THROWS_WITH_AS(mut.add("actor"), "missing mixin name", mutation_error);
    CHECK_THROWS_WITH_AS(mut.to_back("actor"), "missing mixin", mutation_error);

    t.register_all_mixins(dom);

    mut.add(t.mesh->name.to_std());
    mut.add(*t.ai);
    mut.add("actor");

    CHECK_FALSE(mut.noop());
    CHECK(mut.adding_mixins());
    CHECK_FALSE(mut.removing_mixins());

    CHECK(mut.adding("ai"));
    CHECK(mut.adding(*t.actor));
    CHECK(mut.adding(*t.mesh));

    CHECK(nt.has(*t.ai));
    CHECK_FALSE(nt.has(*t.empty));
    CHECK(nt.lacks(*t.empty));
    CHECK(nt.implements_strong(*t.serialize));
    CHECK(nt.implements_strong("render"));

    // normalization
    mut.add("mesh");
    mut.dedup_new_type();
    CHECK(mut.new_type().mixins.size() == 3);
    CHECK(mut.new_type().mixins.back() == t.mesh);

    mut.mod_new_type().add(*t.mesh);
    CHECK(nt.mixins.size() == 4);
    mut.dedup_new_type();
    CHECK(mut.new_type().mixins.size() == 3);
    CHECK(nt.mixins.size() == 3);
}

TEST_CASE("type_mutation from type") {
    test_data t;
    domain dom;
    t.register_all_mixins(dom);
    t.create_types(dom);

    {
        type_mutation mut(*t.t_asim);
        CHECK(mut.noop());
        CHECK_FALSE(mut.adding_mixins());
        CHECK_FALSE(mut.removing_mixins());
        CHECK(mut.new_type().implements_strong(*t.render));
        CHECK_FALSE(mut.new_type().implements_strong(*t.get_name));
        mut.add_if_lacking(*t.mesh);
        CHECK(mut.noop());
        mut.remove("mesh");
        CHECK(mut.removing(*t.mesh));
        CHECK_FALSE(mut.removing(*t.stats));
        mut.remove(*t.stats);
        CHECK(mut.removing(*t.stats));
        CHECK_FALSE(mut.noop());
        CHECK_FALSE(mut.adding_mixins());
        CHECK(mut.removing_mixins());
        CHECK_THROWS_WITH_AS(mut.to_back(*t.stats), "missing mixin", mutation_error);
        CHECK_FALSE(mut.adding("actor"));
        mut.add_if_lacking(*t.actor);
        CHECK(mut.adding("actor"));
        CHECK_FALSE(mut.noop());
        CHECK(mut.adding_mixins());
        CHECK(mut.removing_mixins());
        CHECK(mut.new_type().implements_strong(*t.get_name));
        CHECK_FALSE(mut.new_type().implements_strong(*t.render));

        mut.reset();
        CHECK(mut.noop());
    }

    {
        // reorder
        type_mutation mut(*t.t_asim);
        mut.add(*t.ai);
        CHECK_FALSE(mut.noop());
        CHECK_FALSE(mut.adding_mixins());
        CHECK_FALSE(mut.removing_mixins());

        mut.reset();
        CHECK(mut.noop());
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

        CHECK_FALSE(mut.noop());
        CHECK(mut.adding_mixins());
        CHECK_FALSE(mut.removing_mixins());

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
        CHECK_FALSE(mut.removing_mixins());

        mut.remove(*t.ai);
        CHECK(mut.new_type().has("mesh"));
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

        mut.mod_new_type().mixins = {t.invisible, t.mesh, t.procedural_geometry, t.stats, t.actor};
        CHECK_FALSE(mut.adding_mixins());
        CHECK_FALSE(mut.removing_mixins());
        CHECK_FALSE(mut.noop());

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
        mut.dedup_new_type();
        CHECK_FALSE(mut.noop());
        CHECK(&dom.get_type(mut.new_type().mixins) == t.t_asim);

        mut.remove(*t.stats);
        mut.remove(*t.ai);
        mut.add(*t.actor);
        mut.remove(*t.actor);
        mut.remove(*t.immaterial);
        CHECK_FALSE(mut.adding(*t.actor));
        CHECK(mut.removing(*t.stats));
        CHECK_FALSE(mut.removing(*t.mesh));

        CHECK(&dom.get_type(std::move(mut)) == t.t_m);
    }

    {
        type_mutation mut(dom);
        mut.add(*t.mesh);
        mut.add(*t.stats);
        mut.add(*t.immaterial);
        mut.add(*t.ai);
        CHECK(&dom.get_type(mut.new_type().mixins) == t.t_asim);

        mut.remove(*t.stats);
        mut.remove(*t.ai);
        mut.add(*t.actor);
        mut.remove(*t.actor);
        mut.remove(*t.immaterial);
        CHECK_FALSE(mut.removing(*t.actor));
        CHECK_FALSE(mut.removing(*t.immaterial));
        CHECK(mut.adding("mesh"));
        CHECK(&dom.get_type(mut.new_type().mixins) == t.t_m);

        mut.add(*t.ai);
        mut.add(*t.invisible);
        mut.add(*t.flyer);

        CHECK(mut.adding("mesh"));
        CHECK(mut.adding("flyer"));
        CHECK(&dom.get_type(std::move(mut)) == t.t_afmi);
    }

    {
        t.create_more_types(dom);
        type_mutation mut(*t.t_asmpi);

        mut.mod_new_type().mixins = {t.invisible, t.mesh, t.procedural_geometry, t.stats, t.actor};
        CHECK_FALSE(mut.noop());

        auto& tasmpi = dom.get_type(std::move(mut));
        CHECK(t.t_asmpi == &tasmpi);
    }
}
