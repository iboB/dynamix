// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/exception.hpp>

#include <doctest/doctest.h>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("register features") {
    test_data t;
    domain d;
    auto& ua = *t.update_actor;
    CHECK(invalid_feature_id == ua.id);

    CHECK(ua.name == "update_actor");

    // adding a new feature should occupy slot 0
    d.register_feature(ua);
    CHECK(0 == ua.iid());
    CHECK(d.get_feature_info(feature_id{0}) == &ua);
    CHECK(d.get_feature_info("update_actor") == &ua);

    // remove pos_info
    d.unregister_feature(ua);
    CHECK(invalid_feature_id == ua.id);
    CHECK_FALSE(d.get_feature_info(feature_id{0}));
    CHECK_FALSE(d.get_feature_info("update_actor"));

    // adding it again should place it in slot 0 once more
    d.register_feature(ua);
    CHECK(0 == ua.iid());

    // adding another should place it at slot 1
    auto& gn = *t.get_name;
    CHECK(invalid_feature_id == gn.id);
    d.register_feature(gn);
    CHECK(1 == gn.iid());
    CHECK(d.get_feature_info(feature_id{1}) == &gn);
    CHECK(d.get_feature_info("get_name") == &gn);

    // removing pos should free slot 0
    d.unregister_feature(ua);
    CHECK(invalid_feature_id == ua.id);

    // adding a new type must add it to slot 0
    auto& ec = *t.execute_command;
    CHECK(invalid_feature_id == ec.id);
    d.register_feature(ec);
    CHECK(0 == ec.iid());

    // adding more types should be safe
    d.register_feature(ua);
    CHECK(2 == ua.iid());
    CHECK(d.get_feature_info(feature_id{2}) == &ua);
    CHECK(d.get_feature_info("update_actor") == &ua);

    {
        // duplicate names are not allowed
        feature_info ua2 = dnmx_make_feature_info();
        CHECK_THROWS_WITH_AS(d.register_feature(ua2), "empty info name", domain_error);
        ua2.name = dnmx_make_sv_lit("update_actor");
        CHECK_THROWS_WITH_AS(d.register_feature(ua2), "duplicate name", domain_error);
        CHECK(invalid_feature_id == ua2.id);
    }
}

TEST_CASE("register dupes features") {
    domain_settings s = {};
    s.allow_duplicate_feature_names = true;
    domain d({}, s);

    feature_info e = dnmx_make_feature_info();
    d.register_feature(e);
    CHECK(e.iid() == 0);

    feature_info e2 = dnmx_make_feature_info();
    d.register_feature(e2);
    CHECK(e2.iid() == 1);

    feature_info aa = dnmx_make_feature_info();
    aa.name = dnmx_make_sv_lit("aa");
    d.register_feature(aa);
    CHECK(aa.iid() == 2);

    feature_info aa2 = dnmx_make_feature_info();
    aa2.name = dnmx_make_sv_lit("aa");
    d.register_feature(aa2);
    CHECK(aa2.iid() == 3);
}

TEST_CASE("register all features") {
    test_data t;
    domain d;
    t.register_all_features(d);

    CHECK(d.get_feature_info("update_actor") == t.update_actor);
    CHECK(d.get_feature_info("can_move_to") == t.can_move_to);
    CHECK(t.unused_feature->id == invalid_feature_id);

    t.unregister_all_features(d);

    CHECK_FALSE(d.get_feature_info("update_actor"));
    CHECK_FALSE(d.get_feature_info("can_move_to"));
}
