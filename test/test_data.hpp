// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "test_mixin_allocator.hpp"

#include <dynamix/domain.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/common_mixin_info.hpp>
#include <dynamix/common_feature_info.hpp>
#include <dynamix/feature_for_mixin.hpp>
#include <dynamix/type_class.hpp>
#include <dynamix/type.hpp>

#include <dnmx/bits/pp.h>

#include <doctest/util/lifetime_counter.hpp>

#include <vector>
#include <deque>
#include <memory>

class test_data {
public:
    struct feature_data : public dynamix::common_feature_info {
        std::string default_payload_container;
    };

    std::deque<feature_data> features;

    feature_data* mf(std::string_view name) {
        auto& ret = features.emplace_back();
        ret.name = dnmx_sv::from_std(name);
        return &ret;
    }

    feature_data
        * update_actor,
        * render,
        * update_physics,
        * is_controllable,
        * serialize,
        * move_to,
        * can_move_to,
        * execute_command,
        * get_hp,
        * get_name,
        * get_team,
        * take_damage,
        * set_position,
        * get_position,
        * set_direction,
        * get_direction,
        * go_up_down,
        * feature_of_unused,
        * unused_feature;

    void register_all_features(dynamix::domain& dom) {
        for (auto& f : features) {
            if (f.name == "unused_feature") continue;
            if (f.id != dynamix::invalid_feature_id) continue;
            dom.register_feature(f);
        }
    }

    void unregister_all_features(dynamix::domain& dom) {
        for (auto& f : features) {
            if (f.id == dynamix::invalid_feature_id) continue;
            dom.unregister_feature(f);
        }
    }

    struct m_movable : public doctest::util::lifetime_counter<m_movable> {
        int position = 5;
        bool operator==(const m_movable& o) const noexcept { return position == o.position; }
        int cmp(const m_movable& o) const noexcept {
            if (position == o.position) return 0;
            return position < o.position ? -1 : 1;
        }
    };
    struct m_fixed : public doctest::util::lifetime_counter<m_fixed> {
        std::vector<int> fixed_data;
        m_fixed() = default;
        m_fixed(const m_fixed&) = delete;
        m_fixed& operator=(const m_fixed&) = delete;
        m_fixed(m_fixed&&) noexcept = delete;
        m_fixed& operator=(m_fixed&&) noexcept = delete;
    };
    struct m_actor : public doctest::util::lifetime_counter<m_actor> {
        std::string name = "actor";
        int team = -1;
        m_actor(std::string_view n, int t) : name(n), team(t) {}
        m_actor(m_actor&& other) = default;
        m_actor& operator=(m_actor&& other) = default;
        m_actor(const m_actor& other)
            : doctest::util::lifetime_counter<m_actor>(other)
            , name(other.name)
            , team(other.team)
        {
            if (name == "error") throw std::logic_error("actor cc error");
        }
        m_actor& operator=(const m_actor& other)
        {
            if (other.name == "error") throw std::logic_error("actor asgn error");

            doctest::util::lifetime_counter<m_actor>::operator=(other);
            name = other.name;
            team = other.team;
            return *this;
        }

        bool operator==(const m_actor& o) const noexcept {
            return name == o.name && team == o.team;
        }

        int cmp(const m_actor& o) const noexcept {
            if (name == o.name) {
                if (team == o.team) return 0;
                return team < o.team ? -1 : 1;
            }
            return name < o.name ? -1 : 1;
        }
    };
    struct m_ai : public doctest::util::lifetime_counter<m_ai> {
        bool aggresive = true;
        std::string strategy = "attack";
    };
    struct m_controlled : public doctest::util::lifetime_counter<m_controlled> {
        std::string keyboard;
        std::string mouse;
    };
    struct m_flyer : public doctest::util::lifetime_counter<m_flyer> {
        float wingspan = 3.f;
    };
    struct m_walker : public doctest::util::lifetime_counter<m_walker> {
        double speed = 12.;
    };
    struct m_stats : public doctest::util::lifetime_counter<m_stats> {
        int hp = 0;
        int mp = 0;
        int str = 0;
    };
    struct m_invulnerable : public doctest::util::lifetime_counter<m_invulnerable> {
    };
    struct m_physical : public doctest::util::lifetime_counter<m_physical> {
        std::vector<double> data;
        m_physical(std::vector<double> d) : data(std::move(d)) {}
        bool operator==(const m_physical& other) const noexcept { return data == other.data; }
    };
    struct m_immaterial : public doctest::util::lifetime_counter<m_immaterial> {
        char i = 'i';
    };
    struct m_mesh : public doctest::util::lifetime_counter<m_mesh> {
        m_mesh(size_t x) {
            bb = float(x);
            vertices.resize(x, 3.f);
        }
        m_mesh(const m_mesh&) = default;
        m_mesh& operator=(const m_mesh&) = default;
        m_mesh(m_mesh&&) noexcept = default;
        m_mesh& operator=(m_mesh&&) noexcept = default;
        float bb = 8;
        std::vector<float> vertices;
    };
    struct m_procedural_geometry : public doctest::util::lifetime_counter<m_procedural_geometry> {
        std::string algo;
        float a1 = 4, a2 = 3;

        m_procedural_geometry() = default;
        m_procedural_geometry(const m_procedural_geometry&) = delete;
        m_procedural_geometry& operator=(const m_procedural_geometry&) = delete;
        m_procedural_geometry(m_procedural_geometry&&) noexcept = default;
        m_procedural_geometry& operator=(m_procedural_geometry&&) noexcept = default;

        bool operator==(const m_procedural_geometry& o) const noexcept {
            return algo == o.algo;
        }
        int cmp(const m_procedural_geometry& o) const noexcept {
            if (algo == o.algo) return 0;
            return algo < o.algo ? -1 : 1;
        }
    };
    struct m_invisible : public doctest::util::lifetime_counter<m_invisible> {
        bool active = true;
    };
    struct m_over_under : public doctest::util::lifetime_counter<m_over_under> {
        std::string_view something = "123";
        int other = 5;
    };

    std::deque<dynamix::util::mixin_info_data> mixins;

    struct feature_data_with_perks {
        feature_data_with_perks(const feature_data* i, int b = 0, int p = 0)
            : info(i), bid(b), prio(p)
        {}
        const feature_data* info;
        int bid;
        int prio;
    };

    template <typename M>
    dynamix::util::mixin_info_data& mm(std::string_view name, std::vector<feature_data_with_perks> mfeatures) {
        auto& ret = mixins.emplace_back();
        dynamix::util::mixin_info_data_builder<M> builder(ret, name);
        for (auto& f : mfeatures) {
            std::string pl(ret.info.name.to_std());
            pl += '-';
            pl += f.info->name.to_std();
            builder.implements_with(*f.info, std::move(pl), dynamix::util::builder_literals::feature_bid_priority(f.bid, f.prio));
        }
        return ret;
    }

    template <typename M>
    static int def_cmp(const dynamix::mixin_info*, const void* a, const void* b) noexcept {
        auto ma = static_cast<const M*>(a);
        auto mb = static_cast<const M*>(b);
        return ma->cmp(*mb);
    }

    dynamix::common_mixin_info
        * movable,
        * fixed,
        * actor,
        * ai,
        * controlled,
        * flyer,
        * walker,
        * stats,
        * invulnerable,
        * physical,
        * immaterial,
        * mesh,
        * procedural_geometry,
        * invisible,
        * empty,
        * empty_with_features,
        * over_under,
        * unused;

    void register_all_mixins(dynamix::domain& dom) {
        for (auto& f : mixins) {
            if (f.info.name == "unused") continue;
            if (f.info.id != dynamix::invalid_mixin_id) continue;
            dom.register_mixin(f.info);
        }
    }

    void unregister_all_mixins(dynamix::domain& dom) {
        for (auto& f : mixins) {
            if (f.info.id == dynamix::invalid_mixin_id) continue;
            dom.unregister_mixin(f.info);
        }
    }

    dynamix::type_class
        renderable,
        serializable;

    test_mixin_allocator m_mixin_alloc;

    test_data() {
        using namespace dynamix;

#define MF(f) f = mf(#f)
        MF(update_actor);
        MF(render);
        render->allow_clashes = true;
        MF(update_physics);
        MF(is_controllable);
        MF(serialize);
        serialize->allow_clashes = true;
        serialize->default_payload_container = "default-serialize";
        serialize->default_payload = &serialize->default_payload_container;
        MF(move_to);
        MF(can_move_to);
        MF(execute_command);
        MF(get_hp);
        MF(get_name);
        MF(get_team);
        MF(take_damage);
        take_damage->allow_clashes = true;
        MF(set_position);
        MF(get_position);
        MF(set_direction);
        MF(get_direction);
        MF(go_up_down);
        MF(feature_of_unused);
        MF(unused_feature);

#define VMM(m, ...) m = &mm<void>(#m, __VA_ARGS__).info; auto& desc = *m
#define MM(m, ...) m = &mm<I_DNMX_PP_CAT(m_, m)>(#m, __VA_ARGS__).info; auto& desc = *m

        {
            MM(movable, {move_to});
            desc.user_data = 53;
            desc.order_priority = -2000;
            desc.equals = util::call_mixin_equality_operator<m_movable>;
            desc.compare = def_cmp<m_movable>;
        }
        {
            MM(fixed, {});
            desc.order_priority = -1900;
        }
        {
            MM(actor, {get_name, get_team, execute_command, {serialize, 0, 10}, is_controllable, {go_up_down, 1, 1}});
            desc.order_priority = -1800;
            desc.equals = util::call_mixin_equality_operator<m_actor>;
            desc.compare = def_cmp<m_actor>;
        }
        {
            MM(ai, {update_actor});
            desc.allocator = m_mixin_alloc.to_c_hanlde();
            desc.order_priority = -1800;
        }
        {
            MM(controlled, {update_actor});
            desc.user_data = 72;
            desc.order_priority = -1800;
        }
        {
            MM(flyer, {can_move_to});
            desc.set_size_alignment(sizeof(flyer), 16);
            desc.order_priority = -1800;
        }
        {
            MM(walker, {can_move_to});
            desc.order_priority = -1800;
        }
        {
            MM(stats, {get_hp, take_damage, serialize, {go_up_down, 1, -5}});
            desc.order_priority = -1700;
        }
        {
            MM(invulnerable, {take_damage});
            desc.order_priority = -1600;
        }
        {
            MM(physical, {get_position, set_position, get_direction, set_direction, update_physics});
            desc.allocator = m_mixin_alloc.to_c_hanlde();
            desc.order_priority = -1500;
            desc.equals = util::call_mixin_equality_operator<m_physical>;
        }
        {
            MM(immaterial, {update_physics});
            desc.order_priority = -1400;
        }
        {
            MM(mesh, {{render, 0, -2}, serialize, {go_up_down, -1, 1}});
            desc.set_size_alignment(sizeof(m_mesh), 32);
            desc.init = [](const mixin_info* info, void* ptr) -> error_return_t {
                if (info->user_data == 100) throw std::logic_error("mesh init");
                if (info->user_data == 200) return 1000;
                new (ptr) m_mesh(5);
                return result_success;
            };
            desc.copy_init = [](const mixin_info* info, void* ptr, const void* src) -> error_return_t {
                if (info->user_data == 300) return 10;
                return util::call_mixin_copy_ctor<m_mesh>(info, ptr, src);
            };
            desc.copy_asgn = [](const dynamix::mixin_info* info, void* ptr, const void* src) -> dynamix::error_return_t {
                if (info->user_data == 300) return 100;
                return util::call_mixin_copy_assignment<m_mesh>(info, ptr, src);
            };
            desc.order_priority = -1300;
        }
        {
            MM(procedural_geometry, {render, {go_up_down, -10, 1}});
            desc.force_external = true;
            desc.order_priority = -1200;
            desc.compare = def_cmp<m_procedural_geometry>;
        }
        {
            MM(invisible, {render, {go_up_down, -10, -5}});
            desc.order_priority = -1100;
        }
        {
            VMM(empty, {});
            desc.order_priority = -1000;
        }
        {
            VMM(empty_with_features, {update_actor, update_physics});
            desc.order_priority = 500;
        }
        {
            MM(over_under, {{render, 10}, {is_controllable, 77}, {get_name, -10}, {go_up_down, -1, -5}});
            desc.order_priority = 1000;
        }
        {
            VMM(unused, {feature_of_unused});
            (void)desc;
        }

        renderable.name = dnmx_make_sv_lit("renderable");
        renderable.matches = [](dnmx_type_handle th) noexcept {
            auto t = type::from_c_handle(th);
            return t->implements_strong("render");
        };
        serializable.name = dnmx_make_sv_lit("serializable");
        serializable.matches = [](dnmx_type_handle th) noexcept {
            auto t = type::from_c_handle(th);
            return t->implements_strong("serialize");
        };
    }

    struct lifetimes {
        m_movable::lifetime_stats movable;
        m_fixed::lifetime_stats fixed;
        m_actor::lifetime_stats actor;
        m_ai::lifetime_stats ai;
        m_controlled::lifetime_stats controlled;
        m_flyer::lifetime_stats flyer;
        m_walker::lifetime_stats walker;
        m_stats::lifetime_stats stats;
        m_invulnerable::lifetime_stats invulnerable;
        m_physical::lifetime_stats physical;
        m_immaterial::lifetime_stats immaterial;
        m_mesh::lifetime_stats mesh;
        m_procedural_geometry::lifetime_stats procedural_geometry;
        m_invisible::lifetime_stats invisible;
        m_over_under::lifetime_stats over_under;
    };

    struct lt_sentry {
        std::vector<doctest::util::lifetime_counter_sentry> s;
        lt_sentry(lifetimes& lf) : s({
            {lf.movable}, {lf.fixed}, {lf.actor}, {lf.ai}, {lf.controlled}, {lf.flyer},
            {lf.walker}, {lf.stats}, {lf.invulnerable}, {lf.physical}, {lf.immaterial},
            {lf.mesh}, {lf.procedural_geometry}, {lf.invisible}, {lf.over_under},
            }) {}
    };

    const dynamix::type* t_mov = nullptr;
    const dynamix::type* t_asim = nullptr;
    const dynamix::type* t_m = nullptr;
    const dynamix::type* t_afmi = nullptr;

    void create_types(dynamix::domain& dom) {
        using namespace dynamix;
        {
            const mixin_info* desc[] = {movable};
            t_mov = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {ai, stats, immaterial, mesh};
            t_asim = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {mesh};
            t_m = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {ai, flyer, mesh, invisible};
            t_afmi = &dom.get_type(desc);
        }
    }

    const dynamix::type* t_acp = nullptr;
    const dynamix::type* t_ap = nullptr;
    const dynamix::type* t_pp = nullptr;
    const dynamix::type* t_siee = nullptr;
    const dynamix::type* t_fe = nullptr;
    const dynamix::type* t_apio = nullptr;
    const dynamix::type* t_asmpi = nullptr;

    void create_more_types(dynamix::domain& dom) {
        using namespace dynamix;
        {
            const mixin_info* desc[] = {actor, controlled, physical};
            t_acp = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {actor, procedural_geometry};
            t_ap = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {physical, procedural_geometry};
            t_pp = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {stats, invulnerable, empty, empty_with_features};
            t_siee = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {fixed, empty};
            t_fe = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {actor, procedural_geometry, invisible, over_under};
            t_apio = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {actor, stats, mesh, procedural_geometry, invisible};
            t_asmpi = &dom.get_type(desc);
        }
    }

    const dynamix::type* t_impsa = nullptr;
    const dynamix::type* t_oipa = nullptr;

    void create_reordered_types(dynamix::domain& dom) {
        using namespace dynamix;
        {
            const mixin_info* desc[] = {over_under, invisible, procedural_geometry, actor};
            t_oipa = &dom.get_type(desc);
        }
        {
            const mixin_info* desc[] = {invisible, mesh, procedural_geometry, stats, actor};
            t_impsa = &dom.get_type(desc);
        }
    }
};

std::string& get_payload(dynamix::feature_payload pl) {
    return *reinterpret_cast<std::string*>(pl);
}
