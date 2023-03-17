// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <itlib/qalgorithm.hpp>

// test_data sanity checks

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("test data mixin infos") {
    test_data t;
    std::vector<const dynamix::mixin_info*> mixins = {t.movable, t.fixed, t.actor, t.ai, t.controlled, t.flyer, t.walker, t.stats,
        t.invulnerable, t.physical, t.immaterial, t.mesh, t.procedural_geometry,
        t.invisible, t.empty, t.empty_with_features, t.over_under};

    CHECK(std::is_sorted(mixins.begin(), mixins.end(), dynamix::canonical_mixin_order{}));
}


TEST_CASE("test data mixin types") {
    using mesh = test_data::m_mesh;

    {
        mesh::lifetime_stats ls;
        mesh m1(10);
        CHECK(m1.vertices.size() == 10);
        auto mdata = m1.vertices.data();

        auto m2 = std::move(m1);

        CHECK(m1.vertices.empty());
        CHECK(m2.vertices.size() == 10);
        CHECK(m2.vertices.data() == mdata);

        CHECK(ls.d_ctr == 1);
        CHECK(ls.m_ctr == 1);
        CHECK(ls.living == 2);
        CHECK(ls.total == 2);
        CHECK(m2.vertices.data() == mdata);
    }
    {
        test_data t;
        mesh::lifetime_stats ls;

        mesh m1(10);
        CHECK(m1.vertices.size() == 10);
        auto mdata = m1.vertices.data();

        auto& md = *t.mesh;

        std::aligned_storage_t<sizeof(mesh), alignof(mesh)> storage;
        md.move_init(t.mesh, &storage, &m1);
        auto m2 = reinterpret_cast<mesh*>(&storage);
        CHECK(m1.vertices.empty());
        CHECK(m2->vertices.size() == 10);
        CHECK(m2->vertices.data() == mdata);

        m2->~mesh();
    }
}