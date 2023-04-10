// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/exception.hpp>
#include <dynamix/type.hpp>
#include <dynamix/object_mixin_data.hpp>

#include <doctest/doctest.h>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

struct feature_implementer_data {
    feature_implementer_data(const mixin_info* i, int b = 0, int p = 0)
        : info(i), bid(b), prio(p)
    {}
    const dynamix::mixin_info* info;
    int bid = 0;
    int prio = 0;
};

void test_feature_implementers(const type& type, const dynamix::feature_info& feature, std::vector<feature_implementer_data> impls) {
    auto fte = type.ftable_at(feature.id);
    CHECK(fte);
    REQUIRE(fte.size() == impls.size());
    auto fp = fte.begin;
    auto impl = impls.begin();
    for (; fp != fte.end; ++fp, ++impl) {
        CHECK(fp);
        CHECK(*fp);
        CHECK(fp->mixin_index == type.index_of(impl->info->id));

        std::string expected_pl{impl->info->name.to_std()};
        expected_pl += '-';
        expected_pl += feature.name.to_std();
        CHECK(expected_pl == get_payload(fp->payload));

        auto fpd = fp->data;
        CHECK(fpd->info == &feature);
        CHECK(fpd->payload == fp->payload);
        CHECK(fpd->bid == impl->bid);
        CHECK(fpd->priority == impl->prio);
    }
}

void test_unique_features_in_ftable(const type& type, const mixin_info& info, std::vector<test_data::feature_data_with_perks> features) {
    REQUIRE(type.has(info.name.to_std()));
    for (auto& f : features) {
        test_feature_implementers(type, *f.info, {{&info, f.bid, f.prio}});
    }
}

void test_simple_types(test_data& t, domain& dom) {
    CHECK(t.t_mov->compare(*t.t_afmi) < 0);
    CHECK(t.t_m->compare(*t.t_mov) > 0);
    CHECK(t.t_asim->compare(*t.t_afmi) > 0);
    CHECK(t.t_afmi->compare(*t.t_asim) < 0);

    {
        const dynamix::mixin_info* mov_only[] = {t.movable};
        auto& type_mov = dom.get_type(mov_only);
        REQUIRE(t.t_mov);
        CHECK(t.t_mov == &type_mov);

        // object tests also depend on this
        CHECK(type_mov.copyable());
        CHECK(type_mov.default_constructible());
        CHECK(type_mov.comparable());
        CHECK(type_mov.equality_comparable());

        CHECK(type_mov.ftable_length == t.move_to->iid() + 1);
        for (uint32_t i = 0; i < type_mov.ftable_length; ++i) {
            if (i == t.move_to->iid()) {
                CHECK(type_mov.ftable[i]);
            }
            else {
                CHECK_FALSE(type_mov.ftable[i]);
            }
        }
        CHECK(type_mov.mixins.size() == 1);
        CHECK(type_mov.mixins[0] == t.movable);
        CHECK(type_mov.object_buffer_size ==
            sizeof(object_mixin_data) + sizeof_ptr + sizeof(int)
        );
        CHECK(type_mov.object_buffer_alignment == sizeof_ptr);
        CHECK(type_mov.mixin_offsets.size() == 1);
        CHECK(type_mov.mixin_offsets[0] == sizeof(object_mixin_data) + sizeof_ptr);
        CHECK(type_mov.sparse_mixin_indices.size() == t.movable->iid() + 1);
        CHECK(type_mov.sparse_mixin_indices.back() == 0);
        for (size_t i = 0; i < t.movable->iid(); ++i) {
            CHECK(type_mov.sparse_mixin_indices[i] == invalid_mixin_index);
        }

        CHECK(type_mov.num_objects() == 0);
        CHECK(type_mov.num_mixins() == 1);
        CHECK(type_mov.has(*t.movable));
        CHECK(type_mov.has(t.movable->id));
        CHECK(type_mov.has("movable"));
        CHECK_FALSE(type_mov.has("actor"));
        CHECK_FALSE(type_mov.has(t.controlled->id));
        CHECK(type_mov.implements_strong(*t.move_to));
        CHECK(type_mov.implements_strong("move_to"));
        CHECK(type_mov.implements_strong(t.move_to->id));
        CHECK_FALSE(type_mov.implements_strong("can_move_to"));
        CHECK_FALSE(type_mov.implements_strong(t.get_position->id));
        CHECK_FALSE(type_mov.implements_strong(*t.serialize));
        CHECK(type_mov.implements(*t.serialize)); // default

        {
            auto fte = type_mov.ftable_at(t.serialize->id);
            CHECK_FALSE(fte.begin);
            CHECK_FALSE(fte.top_bid_back);
            CHECK_FALSE(fte.end);
        }

        test_unique_features_in_ftable(type_mov, *t.movable, {t.move_to});
    }

    {
        REQUIRE(t.t_asim);
        auto& type_asim = *t.t_asim;

        // object tests also depend on this
        CHECK(type_asim.copyable());
        CHECK(type_asim.default_constructible());
        CHECK_FALSE(type_asim.comparable());
        CHECK_FALSE(type_asim.equality_comparable());

        CHECK(type_asim.ftable_length >= 6);

        CHECK(type_asim.mixins.size() == 4);
        CHECK(type_asim.mixins[0] == t.ai);
        CHECK(type_asim.mixins[1] == t.stats);
        CHECK(type_asim.mixins[2] == t.immaterial);
        CHECK(type_asim.mixins[3] == t.mesh);

        CHECK(type_asim.object_buffer_alignment == 32);

        if constexpr (sizeof_ptr == 8) {
            CHECK(type_asim.object_buffer_size ==
                4 * sizeof(object_mixin_data)
                + sizeof_ptr + sizeof(int) * 3
                + sizeof(int) + sizeof_ptr + 1
                + 31 + sizeof(test_data::m_mesh)
            );
            CHECK(type_asim.mixin_offsets.size() == 4);
            CHECK(type_asim.mixin_offsets[0] == 0);
            CHECK(type_asim.mixin_offsets[1] == 4 * sizeof(object_mixin_data) + sizeof_ptr);
            CHECK(type_asim.mixin_offsets[2] == type_asim.mixin_offsets[1] + sizeof(int) * 4 + sizeof_ptr);
            CHECK(type_asim.mixin_offsets[3] == type_asim.mixin_offsets[2] + 32);
        }

        CHECK_FALSE(type_asim.sparse_mixin_indices.empty());
        CHECK(type_asim.sparse_mixin_indices.back() != invalid_mixin_index);
        for (size_t i = 0; i < type_asim.sparse_mixin_indices.size(); ++i) {
            auto index = type_asim.sparse_mixin_indices[i];
            if (i == t.ai->iid()) {
                CHECK(index == 0);
            }
            else if (i == t.stats->iid()) {
                CHECK(index == 1);
            }
            else if (i == t.immaterial->iid()) {
                CHECK(index == 2);
            }
            else if (i == t.mesh->iid()) {
                CHECK(index == 3);
            }
            else {
                CHECK(index == invalid_mixin_index);
            }
        }

        CHECK(type_asim.num_objects() == 0);
        CHECK(type_asim.num_mixins() == 4);
        CHECK(type_asim.has(*t.ai));
        CHECK(type_asim.has(t.mesh->id));
        CHECK(type_asim.has("immaterial"));
        CHECK_FALSE(type_asim.has("actor"));
        CHECK_FALSE(type_asim.has(t.controlled->id));
        CHECK(type_asim.index_of("ai") == 0);
        CHECK(type_asim.index_of(t.stats->id) == 1);
        CHECK(type_asim.index_of("immaterial") == 2);
        CHECK(type_asim.index_of(t.mesh->id) == 3);
        CHECK(type_asim.implements_strong(*t.render));
        CHECK(type_asim.implements_strong("serialize"));
        CHECK(type_asim.implements_strong(t.update_physics->id));
        CHECK_FALSE(type_asim.implements_strong("get_position"));
        CHECK_FALSE(type_asim.implements_strong(t.set_position->id));

        {
            auto fte = type_asim.ftable_at(t.set_direction->id);
            CHECK_FALSE(fte.begin);
            CHECK_FALSE(fte.top_bid_back);
            CHECK_FALSE(fte.end);
        }

        test_unique_features_in_ftable(type_asim, *t.ai, {t.update_actor});

        test_unique_features_in_ftable(type_asim, *t.stats, {t.get_hp, t.take_damage});

        test_unique_features_in_ftable(type_asim, *t.mesh, {{t.render, 0, -2}});

        CHECK_FALSE(type_asim.find_next_implementer(*t.unused_feature, *t.mesh));
        CHECK(type_asim.find_next_bidder_set(*t.unused_feature, *t.mesh).empty());
        CHECK_FALSE(type_asim.find_next_implementer(*t.render, *t.mesh));
        CHECK(type_asim.find_next_bidder_set(*t.render, *t.mesh).empty());

        {
            auto fte = type_asim.ftable_at(t.serialize->id);
            CHECK(fte);
            CHECK(fte.begin + 1 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type_asim, *t.serialize, {t.mesh, t.stats});

            {
                auto next = type_asim.find_next_implementer(*t.serialize, *t.mesh);
                CHECK(next);
                CHECK(next == fte.begin + 1);
            }
            CHECK(type_asim.find_next_bidder_set(*t.serialize, *t.mesh).empty());

            CHECK_FALSE(type_asim.find_next_implementer(*t.serialize, *t.stats));
            CHECK(type_asim.find_next_bidder_set(*t.serialize, *t.stats).empty());
        }
    }

    {
        REQUIRE(t.t_afmi);
        auto& type = *t.t_afmi;

        // object tests also depend on this
        CHECK(type.copyable());
        CHECK(type.default_constructible());
        CHECK_FALSE(type.comparable());
        CHECK_FALSE(type.equality_comparable());

        CHECK(type.ftable_length >= 4);

        CHECK(type.mixins.size() == 4);
        CHECK(type.mixins[0] == t.ai);
        CHECK(type.mixins[1] == t.flyer);
        CHECK(type.mixins[2] == t.mesh);
        CHECK(type.mixins[3] == t.invisible);

        CHECK(type.object_buffer_alignment == 32);

        CHECK_FALSE(type.sparse_mixin_indices.empty());
        CHECK(type.sparse_mixin_indices.back() != invalid_mixin_index);
        for (size_t i = 0; i < type.sparse_mixin_indices.size(); ++i) {
            auto index = type.sparse_mixin_indices[i];
            if (i == t.ai->iid()) {
                CHECK(index == 0);
            }
            else if (i == t.flyer->iid()) {
                CHECK(index == 1);
            }
            else if (i == t.mesh->iid()) {
                CHECK(index == 2);
            }
            else if (i == t.invisible->iid()) {
                CHECK(index == 3);
            }
            else {
                CHECK(index == invalid_mixin_index);
            }
        }

        CHECK(type.num_objects() == 0);
        CHECK(type.num_mixins() == 4);
        CHECK(type.has(*t.ai));
        CHECK(type.has("flyer"));
        CHECK(type.has(t.mesh->id));
        CHECK(type.has("invisible"));
        CHECK_FALSE(type.has("actor"));
        CHECK_FALSE(type.has(t.controlled->id));
        CHECK(type.implements_strong(*t.render));
        CHECK(type.implements_strong("can_move_to"));
        CHECK(type.implements_strong(t.update_actor->id));
        CHECK_FALSE(type.implements_strong("get_position"));
        CHECK_FALSE(type.implements_strong(t.set_position->id));

        test_unique_features_in_ftable(type, *t.ai, {t.update_actor});

        test_unique_features_in_ftable(type, *t.flyer, {t.can_move_to});

        {
            auto fte = type.ftable_at(t.render->id);
            CHECK(fte);
            CHECK(fte.begin + 1 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type, *t.render, {{t.mesh, 0, -2}, t.invisible});

            CHECK(type.find_next_bidder_set(*t.render, *t.mesh).empty());
            CHECK(type.find_next_bidder_set(*t.render, *t.invisible).empty());
        }

        test_unique_features_in_ftable(type, *t.mesh, {t.serialize});
    }

    {
        const dynamix::mixin_info* dup[] = {t.movable, t.movable};
        CHECK_THROWS_WITH_AS(dom.get_type(dup),
            "tt: creating type {'movable', 'movable'}: 'movable' duplicate",
            type_error);
    }
}

void test_more_types(test_data& t) {
    CHECK(t.t_ap->compare(*t.t_apio) < 0);
    CHECK(t.t_apio->compare(*t.t_ap) > 0);
    CHECK(t.t_asmpi->compare(*t.t_apio) < 0);
    CHECK(t.t_apio->compare(*t.t_asmpi) > 0);

    {
        auto& type = *t.t_acp;
        CHECK(type.num_mixins() == 3);
        CHECK_FALSE(type.default_constructible());
        CHECK(type.copyable());
        CHECK_FALSE(type.comparable());
        CHECK_FALSE(type.equality_comparable());

        test_unique_features_in_ftable(type, *t.actor, {t.get_name, t.get_team, t.execute_command, {t.serialize, 0, 10}, t.is_controllable, {t.go_up_down, 1, 1}});
        test_unique_features_in_ftable(type, *t.controlled, {t.update_actor});
        test_unique_features_in_ftable(type, *t.physical, {t.get_position, t.set_position, t.get_direction, t.set_direction, t.update_physics});

        CHECK(type.mixin_offsets[2] == 0);
    }

    {
        auto& type = *t.t_ap;
        CHECK(type.num_mixins() == 2);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());
        CHECK(type.comparable());
        CHECK(type.equality_comparable());

        test_unique_features_in_ftable(type, *t.actor, {t.get_name, t.get_team, t.execute_command, {t.serialize, 0, 10}, t.is_controllable});
        test_unique_features_in_ftable(type, *t.procedural_geometry, {t.render});

        {
            auto fte = type.ftable_at(t.go_up_down->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 2 == fte.end);

            test_feature_implementers(type, *t.go_up_down, {{t.actor, 1, 1}, {t.procedural_geometry, -10, 1}});

            {
                auto next_bidders = type.find_next_bidder_set(*t.go_up_down, *t.actor);
                CHECK(next_bidders.size() == 1);
                CHECK(next_bidders[0].mixin_index == 1);
                CHECK(next_bidders[0].data->info == t.go_up_down);
            }

            CHECK(type.find_next_bidder_set(*t.go_up_down, *t.procedural_geometry).empty());
        }

        CHECK(type.mixin_offsets[1] == 0);
    }

    {
        auto& type = *t.t_pp;
        CHECK(type.num_mixins() == 2);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());
        CHECK_FALSE(type.comparable());
        CHECK(type.equality_comparable());

        test_unique_features_in_ftable(type, *t.physical, {t.get_position, t.set_position, t.get_direction, t.set_direction, t.update_physics});
        test_unique_features_in_ftable(type, *t.procedural_geometry, {t.render, {t.go_up_down, -10, 1}});

        CHECK(type.mixin_offsets[0] == 0);
        CHECK(type.mixin_offsets[1] == 0);
    }

    {
        auto& type = *t.t_siee;
        CHECK(type.num_mixins() == 4);
        CHECK(type.default_constructible());
        CHECK(type.copyable());

        test_unique_features_in_ftable(type, *t.stats, {t.get_hp, t.serialize, {t.go_up_down, 1, -5}});
        test_unique_features_in_ftable(type, *t.empty_with_features, {t.update_actor, t.update_physics});

        test_feature_implementers(type, *t.take_damage, {t.invulnerable, t.stats});
    }

    {
        auto& type = *t.t_fe;
        CHECK(type.num_mixins() == 2);
        CHECK(type.default_constructible());
        CHECK_FALSE(type.copyable());
        CHECK(type.ftable_length == 0);
    }

    {
        auto& type = *t.t_apio;
        CHECK(type.num_mixins() == 4);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());
        CHECK_FALSE(type.comparable());
        CHECK_FALSE(type.equality_comparable());

        test_unique_features_in_ftable(type, *t.actor, {t.get_team, t.execute_command, {t.serialize, 0, 10}});

        {
            auto fte = type.ftable_at(t.render->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 3 == fte.end);

            test_feature_implementers(type, *t.render, {{t.over_under, 10, 0}, t.invisible, t.procedural_geometry});

            CHECK_FALSE(type.find_next_implementer(*t.render, *t.actor));
            CHECK_FALSE(type.find_next_implementer(*t.unused_feature, *t.invisible));

            {
                auto next = type.find_next_implementer(*t.render, *t.over_under);
                CHECK(next);
                CHECK(next == fte.begin + 1);
            }

            {
                auto next = type.find_next_implementer(*t.render, *t.invisible);
                CHECK(next);
                CHECK(next == fte.begin + 2);
            }

            CHECK_FALSE(type.find_next_implementer(*t.render, *t.procedural_geometry));

            {
                auto next_bidders = type.find_next_bidder_set(*t.render, *t.over_under);
                CHECK(next_bidders.size() == 2);
                CHECK(next_bidders[0].mixin_index == 2);
                CHECK(next_bidders[1].mixin_index == 1);
                CHECK(next_bidders[0].data->info == t.render);
            }

            CHECK(type.find_next_bidder_set(*t.render, *t.invisible).empty());
        }

        {
            auto fte = type.ftable_at(t.is_controllable->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 2 == fte.end);

            test_feature_implementers(type, *t.is_controllable, {{t.over_under, 77, 0}, t.actor});
        }

        {
            auto fte = type.ftable_at(t.get_name->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 2 == fte.end);

            test_feature_implementers(type, *t.get_name, {t.actor, {t.over_under, -10, 0}});
        }

        {
            auto fte = type.ftable_at(t.go_up_down->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 4 == fte.end);

            test_feature_implementers(type, *t.go_up_down, {{t.actor, 1, 1}, {t.over_under, -1, -5}, {t.invisible, -10, -5}, {t.procedural_geometry, -10, 1}});
        }
    }

    {
        auto& type = *t.t_asmpi;
        CHECK(type.num_mixins() == 5);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());

        test_unique_features_in_ftable(type, *t.actor, {t.get_name, t.get_team, t.execute_command, t.is_controllable});
        test_unique_features_in_ftable(type, *t.stats, {t.get_hp, t.take_damage});

        {
            auto fte = type.ftable_at(t.serialize->id);
            CHECK(fte);
            CHECK(fte.begin + 2 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type, *t.serialize, {t.mesh, t.stats, {t.actor, 0, 10}});
        }

        {
            auto fte = type.ftable_at(t.render->id);
            CHECK(fte);
            CHECK(fte.begin + 2 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type, *t.render, {{t.mesh, 0, -2}, t.invisible, t.procedural_geometry});
        }

        {
            auto fte = type.ftable_at(t.go_up_down->id);
            CHECK(fte);
            CHECK(fte.begin + 1 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 4 == fte.end);

            test_feature_implementers(type, *t.go_up_down, {{t.stats, 1, -5},  {t.actor, 1, 1}, {t.mesh, -1, 1}, {t.invisible, -10, -5}, {t.procedural_geometry, -10, 1}});
        }
    }
}

void test_reordered_types(test_data& t) {

    {
        auto& type = *t.t_oipa;
        CHECK(type.num_mixins() == 4);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());

        CHECK(type.index_of(t.over_under->id) == 0);
        CHECK(type.index_of(t.invisible->id) == 1);
        CHECK(type.index_of(t.procedural_geometry->id) == 2);
        CHECK(type.index_of(t.actor->id) == 3);

        test_unique_features_in_ftable(type, *t.actor, {t.get_team, t.execute_command, {t.serialize, 0, 10}});

        {
            auto fte = type.ftable_at(t.render->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 3 == fte.end);

            test_feature_implementers(type, *t.render, {{t.over_under, 10, 0}, t.procedural_geometry, t.invisible});
        }

        {
            auto fte = type.ftable_at(t.is_controllable->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 2 == fte.end);

            test_feature_implementers(type, *t.is_controllable, {{t.over_under, 77, 0}, t.actor});
        }

        {
            auto fte = type.ftable_at(t.get_name->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 2 == fte.end);

            test_feature_implementers(type, *t.get_name, {t.actor, {t.over_under, -10, 0}});
        }

        {
            auto fte = type.ftable_at(t.go_up_down->id);
            CHECK(fte);
            CHECK(fte.begin == fte.top_bid_back);
            CHECK(fte.top_bid_back + 4 == fte.end);

            test_feature_implementers(type, *t.go_up_down, {{t.actor, 1, 1}, {t.over_under, -1, -5}, {t.invisible, -10, -5}, {t.procedural_geometry, -10, 1}});
        }
    }

    {
        auto& type = *t.t_impsa;
        CHECK(type.num_mixins() == 5);
        CHECK_FALSE(type.default_constructible());
        CHECK_FALSE(type.copyable());

        test_unique_features_in_ftable(type, *t.actor, {t.get_name, t.get_team, t.execute_command, t.is_controllable});
        test_unique_features_in_ftable(type, *t.stats, {t.get_hp, t.take_damage});

        {
            auto fte = type.ftable_at(t.serialize->id);
            CHECK(fte);
            CHECK(fte.begin + 2 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type, *t.serialize, {t.stats, t.mesh, {t.actor, 0, 10}});
        }

        {
            auto fte = type.ftable_at(t.render->id);
            CHECK(fte);
            CHECK(fte.begin + 2 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 1 == fte.end);

            test_feature_implementers(type, *t.render, {{t.mesh, 0, -2}, t.procedural_geometry, t.invisible});
        }

        {
            auto fte = type.ftable_at(t.go_up_down->id);
            CHECK(fte);
            CHECK(fte.begin + 1 == fte.top_bid_back);
            CHECK(fte.top_bid_back + 4 == fte.end);

            test_feature_implementers(type, *t.go_up_down, {{t.stats, 1, -5},  {t.actor, 1, 1}, {t.mesh, -1, 1}, {t.invisible, -10, -5}, {t.procedural_geometry, -10, 1}});
        }
    }

}

TEST_CASE("types") {
    test_data t;
    domain dom("tt");
    t.register_all_mixins(dom);

    CHECK(dom.num_types() == 0);

    t.create_types(dom);
    CHECK(t.t_asim->compare(*t.t_asim) == 0);
    CHECK(*t.t_asim == *t.t_asim);
    CHECK(*t.t_asim != *t.t_afmi);

    CHECK(dom.num_types() == 4);
    CHECK(dom.num_type_queries() == 4);

    test_simple_types(t, dom);

    {
        const dynamix::mixin_info* misa[] = {t.mesh, t.immaterial, t.stats, t.ai};
        auto& t_misa = dom.get_type(misa);
        CHECK(&t_misa != t.t_asim);

        CHECK(t_misa.has(*t.ai));
        CHECK(t_misa.has(t.mesh->id));
        CHECK(t_misa.has("stats"));

        auto fte = t_misa.ftable_at(t.serialize->id);
        CHECK(fte);
        CHECK(fte.begin + 1 == fte.top_bid_back);
        CHECK(fte.top_bid_back + 1 == fte.end);

        test_feature_implementers(t_misa, *t.serialize, {t.stats, t.mesh});
    }

    {
        const dynamix::mixin_info* clash[] = {t.flyer, t.walker};
        CHECK_THROWS_WITH_AS(dom.get_type(clash), "tt: feature clash in {'flyer', 'walker'} on 'can_move_to' between 'walker' and 'flyer'", type_error);
    }

    CHECK(dom.num_types() == 5);
    CHECK(dom.num_type_queries() == 5);
    dom.unregister_mixin(*t.mesh);
    CHECK(dom.num_types() == 1);
    CHECK(dom.num_type_queries() == 1);

    dom.register_mixin(*t.mesh);
    t.create_more_types(dom);
    CHECK(dom.num_types() == 8);
    CHECK(dom.num_type_queries() == 8);
    test_more_types(t);

    t.create_reordered_types(dom);
    CHECK(dom.num_types() == 10);
    CHECK(dom.num_type_queries() == 10);
    test_reordered_types(t);

    dom.garbage_collect_types();
    CHECK(dom.num_types() == 0);
    CHECK(dom.num_type_queries() == 0);
}

TEST_CASE("unregistered mixins") {
    test_data t;
    domain dom("tt");

    CHECK_THROWS_WITH_AS(t.create_types(dom),
        "tt: creating type {'movable'}: 'movable' unregistered",
        type_error);
    CHECK_FALSE(t.t_mov);
    CHECK_FALSE(t.t_asim);
    t.register_all_mixins(dom);

    t.create_types(dom);
    CHECK(dom.num_types() == 4);
    test_simple_types(t, dom);

    CHECK(dom.get_mixin_info("actor"));

    t.create_more_types(dom);
    CHECK(dom.num_types() == 11);
    test_more_types(t);

    t.create_reordered_types(dom);
    CHECK(dom.num_types() == 13);
    test_reordered_types(t);
}

TEST_CASE("types canon") {
    test_data t;
    domain_settings s = {};
    s.canonicalize_types = true;
    domain dom("tt", s);
    t.register_all_mixins(dom);
    t.create_types(dom);
    CHECK(dom.num_types() == 4);

    t.create_more_types(dom);
    CHECK(dom.num_types() == 11);
    CHECK(dom.num_type_queries() == 11);
    test_more_types(t);

    {
        const dynamix::mixin_info* dup[] = {t.mesh, t.immaterial, t.stats, t.ai};
        auto& tdup = dom.get_type(dup);
        CHECK(&tdup == t.t_asim);
    }

    {
        t.create_reordered_types(dom);
        CHECK(t.t_oipa == t.t_apio);
        CHECK(t.t_impsa == t.t_asmpi);
    }

    CHECK(dom.num_types() == 11);
    CHECK(dom.num_type_queries() == 14);
}
