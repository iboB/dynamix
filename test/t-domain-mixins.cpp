// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/exception.hpp>
#include <dynamix/domain.from_info.hpp>

#include <doctest/doctest.h>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("reg/unreg mixin") {
    test_data t;
    CHECK(t.movable->user_data == 53);

    domain dom("test", {}, 0, &t);
    t.register_all_features(dom);

    CHECK_FALSE(domain_from_info(*t.movable));

    // adding a new mixin type info should occupy slot 0
    CHECK(invalid_mixin_id == t.movable->id);
    dom.register_mixin(*t.movable);
    CHECK(0 == t.movable->iid());
    CHECK(t.movable->registered());
    CHECK(domain_from_info(*t.movable) == &dom);
    CHECK(dom.get_mixin_info(t.movable->id) == t.movable);

    // remove t.movable
    dom.unregister_mixin(*t.movable);
    CHECK(invalid_mixin_id == t.movable->id);
    CHECK_FALSE(dom.get_mixin_info(mixin_id{0}));

    // adding it again should place it in slot 0 once more
    dom.register_mixin(*t.movable);
    CHECK(0 == t.movable->iid());

    // adding another should place it at slot 1
    CHECK(invalid_mixin_id == t.actor->id);
    dom.register_mixin(*t.actor);
    CHECK(1 == t.actor->iid());

    // removing pos should free slot 0
    dom.unregister_mixin(*t.movable);
    CHECK(invalid_mixin_id == t.movable->id);

    // adding a new type must add it to slot 0
    CHECK(invalid_mixin_id == t.invulnerable->id);
    dom.register_mixin(*t.invulnerable);
    CHECK(0 == t.invulnerable->iid());

    // adding more types should be safe
    dom.register_mixin(*t.movable);
    CHECK(2 == t.movable->iid());

    {
        // duplicate names are not allowed
        mixin_info m2 = dnmx_make_mixin_info();
        m2.name = dnmx_sv::from_std("movable");
        CHECK_THROWS_WITH_AS(dom.register_mixin(m2), "test: register mixin with duplicate name 'movable'", domain_error);
    }

    {
        // empty names are not allowed
        mixin_info e = dnmx_make_mixin_info();
        CHECK_THROWS_WITH_AS(dom.register_mixin(e), "test: register mixin with empty name", domain_error);
    }
}

TEST_CASE("auto-reg features") {
    test_data t;
    domain dom;

    auto check_reg = [&](const feature_info* info) {
        CHECK(invalid_feature_id != info->id);
        CHECK(dom.get_feature_info(info->id) == info);
    };
    auto check_unreg = [&](const feature_info* info) {
        CHECK(invalid_feature_id == info->id);
        CHECK(dom.get_feature_info(info->name.to_std()) == nullptr);
    };

    dom.register_mixin(*t.actor);

    check_reg(t.get_name);
    check_reg(t.get_team);
    check_reg(t.execute_command);
    check_reg(t.serialize);
    check_reg(t.is_controllable);

    check_unreg(t.update_actor);
    check_unreg(t.render);
    check_unreg(t.update_physics);
    check_unreg(t.move_to);
    check_unreg(t.can_move_to);
    check_unreg(t.get_hp);
    check_unreg(t.take_damage);
    check_unreg(t.set_position);
    check_unreg(t.get_position);
    check_unreg(t.set_direction);
    check_unreg(t.get_direction);
    check_unreg(t.feature_of_unused);
    check_unreg(t.unused_feature);

    dom.unregister_mixin(*t.actor);
    dom.register_mixin(*t.stats);

    check_reg(t.get_name);
    check_reg(t.get_team);
    check_reg(t.execute_command);
    check_reg(t.serialize);
    check_reg(t.is_controllable);
    check_reg(t.get_hp);
    check_reg(t.take_damage);

    check_unreg(t.update_actor);
    check_unreg(t.render);
    check_unreg(t.update_physics);
    check_unreg(t.move_to);
    check_unreg(t.can_move_to);
    check_unreg(t.set_position);
    check_unreg(t.get_position);
    check_unreg(t.set_direction);
    check_unreg(t.get_direction);
    check_unreg(t.feature_of_unused);
    check_unreg(t.unused_feature);

    t.register_all_mixins(dom);

    check_reg(t.get_name);
    check_reg(t.get_team);
    check_reg(t.execute_command);
    check_reg(t.serialize);
    check_reg(t.is_controllable);
    check_reg(t.get_hp);
    check_reg(t.take_damage);
    check_reg(t.update_actor);
    check_reg(t.render);
    check_reg(t.update_physics);
    check_reg(t.move_to);
    check_reg(t.can_move_to);
    check_reg(t.set_position);
    check_reg(t.get_position);
    check_reg(t.set_direction);
    check_reg(t.get_direction);

    check_unreg(t.feature_of_unused);
    check_unreg(t.unused_feature);
}

TEST_CASE("register dupe mixins") {
    test_data t;
    domain_settings s = {};
    s.allow_duplicate_mixin_names = true;
    domain dom({}, s);

    t.register_all_mixins(dom);
    CHECK(dom.get_mixin_info("movable") == t.movable);

    mixin_info dup = dnmx_make_mixin_info();
    dup.name = dnmx_sv::from_std("movable");
    dom.register_mixin(dup);

    CHECK(dom.get_mixin_info(t.movable->id) == t.movable);
    CHECK(dom.get_mixin_info(dup.id) == &dup);
}
