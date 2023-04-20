// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/exception.hpp>
#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/mutate_to.hpp>
#include <dynamix/object_mutation.hpp>
#include <dynamix/object_of.hpp>

#include <doctest/doctest.h>

#include <optional>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("empty object") {
    domain dom;
    object obj(dom);
    CHECK(&obj.get_domain() == &dom);
    CHECK(obj.empty());
    CHECK(obj.num_mixins() == 0);
    CHECK(obj.get_type() == dom.get_empty_type());
    CHECK_FALSE(obj.sealed());
    obj.clear(); // should be safe
    CHECK(obj.get_type() == dom.get_empty_type());

    auto o2 = obj.copy();
    CHECK(o2.empty());
    CHECK(o2.num_mixins() == 0);
    CHECK(&o2.get_domain() == &dom);
    CHECK(o2.get_type() == dom.get_empty_type());

    auto o3 = std::move(obj);
    CHECK(o3.empty());
    CHECK(&o3.get_domain() == &dom);
    CHECK(o3.get_type() == dom.get_empty_type());

    o2.move_matching_from(o3);
    CHECK(o2.empty());
    CHECK(o3.empty());

    o2.copy_matching_from(o3);
    CHECK(o2.empty());

    o2.reset_type();
    CHECK(o2.empty());

    o2.reset_type(dom.get_empty_type());
    CHECK(o2.empty());
}

#define qget(obj, type) reinterpret_cast<I_DNMX_PP_CAT(test_data::m_, type)*>(obj.get(*t. type))
#define cqget(obj, type) reinterpret_cast<I_DNMX_PP_CAT(const test_data::m_, type)*>(obj.get(*t. type))

TEST_CASE("simple object") {
    test_data t;
    domain dom("s");
    t.register_all_mixins(dom);
    t.create_types(dom);

    test_data::lifetimes lstats;
    test_data::lt_sentry _ls(lstats);

    object mov(*t.t_mov);
    CHECK(&mov.get_domain() == &dom);
    CHECK(mov.get_type() == *t.t_mov);
    CHECK_FALSE(mov.empty());
    CHECK(mov.num_mixins() == 1);
    CHECK(mov.has(*t.movable));
    CHECK(mov.has(t.movable->id));
    CHECK(mov.has("movable"));
    CHECK(t.t_mov->num_objects() == 1);
    CHECK(mov.equals(mov));
    CHECK(mov.compare(mov) == 0);

    {
        auto vm = mov.get(*t.movable);
        CHECK(vm);
        CHECK(mov.get(t.movable->id) == vm);
        CHECK(mov.get("movable") == vm);
        CHECK(mov.get_at(0) == vm);
        CHECK(mov.unchecked_get_at(0) == vm);
        CHECK(object_of(vm) == &mov);
        auto* m = reinterpret_cast<test_data::m_movable*>(vm);
        CHECK(object_of(m) == &mov);
        CHECK(m->position == 5);
        m->position = 10;
    }

    CHECK_FALSE(mov.get("mesh"));
    CHECK_FALSE(mov.get(t.ai->id));
    CHECK_FALSE(mov.get_at(1));

    CHECK(lstats.movable.d_ctr == 1);
    CHECK(lstats.movable.living == 1);
    CHECK(lstats.movable.total == 1);

    object mcopy = mov.copy();
    CHECK(&mcopy.get_domain() == &dom);
    CHECK(mcopy.get_type() == *t.t_mov);
    CHECK_FALSE(mcopy.empty());
    CHECK(mcopy.has(*t.movable));
    CHECK(mcopy.has(t.movable->id));
    CHECK(mcopy.has("movable"));
    CHECK(t.t_mov->num_objects() == 2);
    CHECK(mov.equals(mcopy));
    CHECK(mov.compare(mcopy) == 0);

    CHECK(lstats.movable.d_ctr == 1);
    CHECK(lstats.movable.c_ctr == 1);
    CHECK(lstats.movable.living == 2);
    CHECK(lstats.movable.total == 2);

    {
        auto vm = mcopy.get(*t.movable);
        CHECK(vm);
        CHECK(object_of(vm) == &mcopy);
        auto* m = reinterpret_cast<test_data::m_movable*>(vm);
        CHECK(m->position == 10);

        m->position = 15;
    }
    CHECK(mov.get_at(0) != mcopy.get_at(0));

    CHECK_FALSE(mov.equals(mcopy));
    CHECK_FALSE(mcopy.equals(mov));
    CHECK(mov.compare(mcopy) < 0);
    CHECK(mcopy.compare(mov) > 0);

    mcopy.clear();
    CHECK(mcopy.empty());
    CHECK(mcopy.num_mixins() == 0);
    CHECK_FALSE(mcopy.has(t.movable->id));
    CHECK(mcopy.get_type() == dom.get_empty_type());
    CHECK(t.t_mov->num_objects() == 1);

    CHECK(lstats.movable.living == 1);
    CHECK(lstats.movable.total == 2);

    object asim(*t.t_asim);
    CHECK(&asim.get_domain() == &dom);
    CHECK(&asim.get_type() == t.t_asim);
    CHECK_FALSE(asim.empty());
    CHECK(asim.num_mixins() == 4);
    CHECK(t.t_asim->num_objects() == 1);

    CHECK(lstats.ai.d_ctr == 1);
    CHECK(lstats.ai.living == 1);
    CHECK(lstats.ai.total == 1);
    CHECK(lstats.stats.d_ctr == 1);
    CHECK(lstats.stats.living == 1);
    CHECK(lstats.stats.total == 1);
    CHECK(lstats.immaterial.d_ctr == 1);
    CHECK(lstats.immaterial.living == 1);
    CHECK(lstats.immaterial.total == 1);
    CHECK(lstats.mesh.d_ctr == 1);
    CHECK(lstats.mesh.living == 1);
    CHECK(lstats.mesh.total == 1);

    CHECK(asim.has(*t.ai));
    CHECK(asim.has("stats"));
    CHECK(asim.has(t.immaterial->id));
    CHECK(asim.has(*t.mesh));

    {
        auto vm = asim.get("ai");
        CHECK(vm == asim.get_at(0));
        CHECK(vm == asim.get(t.ai->id));
        CHECK(vm == asim.get(*t.ai));
        CHECK(object_of(vm) == &asim);
        auto m = reinterpret_cast<test_data::m_ai*>(vm);
        CHECK(m->aggresive);
        CHECK(m->strategy == "attack");
        m->aggresive = false;
        m->strategy = "defend";
    }

    {
        auto vm = asim.get(*t.stats);
        CHECK(vm == asim.get_at(1));
        CHECK(vm == asim.get(t.stats->id));
        CHECK(vm == asim.get("stats"));
        CHECK(object_of(vm) == &asim);
        auto m = reinterpret_cast<test_data::m_stats*>(vm);
        CHECK(m->hp == 0);
        CHECK(m->mp == 0);
        CHECK(m->str == 0);
        m->hp = 10;
        m->mp = 20;
        m->str = 30;
    }

    {
        auto vm = asim.get(*t.immaterial);
        CHECK(vm == asim.get_at(2));
        CHECK(vm == asim.get(t.immaterial->id));
        CHECK(vm == asim.get("immaterial"));
        CHECK(object_of(vm) == &asim);
        auto m = reinterpret_cast<test_data::m_immaterial*>(vm);
        CHECK(m->i == 'i');
        m->i = 'z';
    }

    {
        auto vm = asim.get(*t.mesh);
        CHECK(vm == asim.get_at(3));
        CHECK(vm == asim.get(t.mesh->id));
        CHECK(vm == asim.get("mesh"));
        CHECK(object_of(vm) == &asim);
        auto m = reinterpret_cast<test_data::m_mesh*>(vm);
        CHECK(m->bb == 5.f);
        CHECK(m->vertices == std::vector<float>{3.f, 3.f, 3.f, 3.f, 3.f});
        m->bb = 1.f;
        m->vertices.resize(20, 5.f);
    }

    mcopy.reset_type(*t.t_m);
    CHECK(mcopy.num_mixins() == 1);
    CHECK(&mcopy.get_type() == t.t_m);
    CHECK(t.t_m->num_objects() == 1);

    CHECK(lstats.mesh.d_ctr == 2);
    CHECK(lstats.mesh.living == 2);
    CHECK(lstats.mesh.total == 2);

    float* mcopy_mesh_data;
    {
        auto vm = mcopy.get(*t.mesh);
        CHECK(vm == mcopy.get_at(0));
        CHECK(vm == mcopy.get(t.mesh->id));
        CHECK(vm == mcopy.get("mesh"));
        CHECK(object_of(vm) == &mcopy);
        auto m = reinterpret_cast<test_data::m_mesh*>(vm);
        CHECK(m->bb == 5.f);
        CHECK(m->vertices == std::vector<float>{3.f, 3.f, 3.f, 3.f, 3.f});
        m->bb = 10.f;
        m->vertices.clear();
        m->vertices.resize(50, 1.f);
        mcopy_mesh_data = m->vertices.data();
    }

    mcopy.copy_from(asim);
    CHECK(mcopy.num_mixins() == 4);
    CHECK(&mcopy.get_domain() == &dom);
    CHECK(mcopy.get_type() == *t.t_asim);
    CHECK(t.t_m->num_objects() == 0);
    CHECK(t.t_asim->num_objects() == 2);

    CHECK(lstats.ai.d_ctr == 1);
    CHECK(lstats.ai.c_ctr == 1);
    CHECK(lstats.ai.living == 2);
    CHECK(lstats.ai.total == 2);
    CHECK(lstats.stats.d_ctr == 1);
    CHECK(lstats.stats.c_ctr == 1);
    CHECK(lstats.stats.living == 2);
    CHECK(lstats.stats.total == 2);
    CHECK(lstats.immaterial.d_ctr == 1);
    CHECK(lstats.immaterial.c_ctr == 1);
    CHECK(lstats.immaterial.living == 2);
    CHECK(lstats.immaterial.total == 2);
    CHECK(lstats.mesh.d_ctr == 2);
    CHECK(lstats.mesh.c_asgn == 1);
    CHECK(lstats.mesh.m_ctr == 1);
    CHECK(lstats.mesh.living == 2);
    CHECK(lstats.mesh.total == 3);

    void* mcopy_ai;
    {
        mcopy_ai = mcopy.get("ai");
        CHECK(object_of(mcopy_ai) == &mcopy);
        auto m = reinterpret_cast<test_data::m_ai*>(mcopy_ai);
        CHECK_FALSE(m->aggresive);
        CHECK(m->strategy == "defend");
    }

    {
        auto vm = mcopy.get(*t.stats);
        CHECK(object_of(vm) == &mcopy);
        auto m = reinterpret_cast<test_data::m_stats*>(vm);
        CHECK(m->hp == 10);
        CHECK(m->mp == 20);
        CHECK(m->str == 30);
    }

    {
        auto vm = mcopy.get_at(2);
        CHECK(object_of(vm) == &mcopy);
        auto m = reinterpret_cast<test_data::m_immaterial*>(vm);
        CHECK(m->i == 'z');
    }

    void* mcopy_mesh;
    {
        mcopy_mesh = mcopy.get(t.mesh->id);
        CHECK(object_of(mcopy_mesh) == &mcopy);
        auto m = reinterpret_cast<test_data::m_mesh*>(mcopy_mesh);
        CHECK(m->bb == 1.f);
        CHECK(m->vertices.size() == 20);
        CHECK(m->vertices[1] == 3.f);
        CHECK(m->vertices[11] == 5.f);
        CHECK(m->vertices.data() == mcopy_mesh_data);
    }

    mcopy.reset_type(*t.t_afmi);
    CHECK(mcopy.num_mixins() == 4);
    CHECK(&mcopy.get_type() == t.t_afmi);
    CHECK(t.t_asim->num_objects() == 1);
    CHECK(t.t_afmi->num_objects() == 1);
    CHECK_FALSE(mcopy.equals(asim));
    CHECK_FALSE(mcopy.compare(asim) > 0);
    CHECK_FALSE(asim.compare(mcopy) < 0);

    CHECK(mcopy_ai == mcopy.get(*t.ai)); // external
    CHECK(qget(mcopy, ai)->strategy == "defend");
    CHECK(mcopy_mesh != mcopy.get(*t.mesh)); // internal
    mcopy_mesh = mcopy.get(*t.mesh);
    CHECK(mcopy_mesh_data == qget(mcopy, mesh)->vertices.data());
    qget(mcopy, ai)->strategy = "something entirely different but also long";
    {
        qget(mcopy, mesh)->bb = 3.f;
        std::vector<float> v123 = {1.f, 2.f, 3.f};
        qget(mcopy, mesh)->vertices.swap(v123);
    }
    mcopy_mesh_data = qget(mcopy, mesh)->vertices.data();

    CHECK(lstats.ai.copies == 1);
    CHECK(lstats.ai.living == 2);
    CHECK(lstats.ai.total == 2);
    CHECK(lstats.flyer.d_ctr == 1);
    CHECK(lstats.flyer.living == 1);
    CHECK(lstats.flyer.total == 1);
    CHECK(lstats.stats.living == 1);
    CHECK(lstats.stats.total == 2);
    CHECK(lstats.immaterial.living == 1);
    CHECK(lstats.immaterial.total == 2);
    CHECK(lstats.mesh.d_ctr == 2);
    CHECK(lstats.mesh.c_asgn == 1);
    CHECK(lstats.mesh.m_ctr == 2);
    CHECK(lstats.mesh.living == 2);
    CHECK(lstats.mesh.total == 4);
    CHECK(lstats.invisible.d_ctr == 1);
    CHECK(lstats.invisible.living == 1);
    CHECK(lstats.invisible.total == 1);

    asim.copy_matching_from(mcopy);
    CHECK(&asim.get_type() == t.t_asim);
    CHECK(qget(asim, mesh)->bb == 3.f);
    CHECK(qget(asim, mesh)->vertices == std::vector<float>{1.f, 2.f, 3.f});
    CHECK(qget(asim, ai)->strategy == "something entirely different but also long");

    CHECK(lstats.ai.copies == 2);
    CHECK(lstats.ai.living == 2);
    CHECK(lstats.ai.total == 2);
    CHECK(lstats.mesh.c_asgn == 2);
    CHECK(lstats.mesh.living == 2);
    CHECK(lstats.mesh.total == 4);

    qget(asim, mesh)->vertices.clear();
    qget(asim, ai)->strategy = "short";
    asim.move_matching_from(mcopy);
    CHECK(&asim.get_type() == t.t_asim);
    CHECK(mcopy_mesh == mcopy.get(*t.mesh)); // no buf change
    CHECK(mcopy_mesh_data == qget(asim, mesh)->vertices.data());
    CHECK(qget(asim, ai)->strategy == "something entirely different but also long");

    CHECK(qget(mcopy, ai)->strategy.empty());
    CHECK(qget(mcopy, mesh)->vertices.empty());

    auto mmove = std::move(mcopy);
    CHECK(&mmove.get_type() == t.t_afmi);
    CHECK(mcopy_mesh == mmove.get(*t.mesh)); // no buf change
    CHECK(mcopy.empty());
    CHECK(mcopy.get_type() == dom.get_empty_type());
    CHECK(t.t_afmi->num_objects() == 1);

    auto move2 = asim.copy();
    CHECK(&move2.get_type() == t.t_asim);
    CHECK(t.t_asim->num_objects() == 2);

    move2 = std::move(mmove);
    CHECK(t.t_asim->num_objects() == 1);
    CHECK(t.t_afmi->num_objects() == 1);

    CHECK(&move2.get_type() == t.t_afmi);
    CHECK(mcopy_mesh == move2.get(*t.mesh)); // no buf change
    CHECK(mmove.empty());
    CHECK(mmove.get_type() == dom.get_empty_type());

    object asim2(*t.t_asim);
    CHECK(&asim2.get_type() == t.t_asim);
    CHECK(t.t_asim->num_objects() == 2);
    asim2.copy_from(asim);
    CHECK(&asim2.get_type() == t.t_asim);
    CHECK(qget(asim2, mesh)->vertices == qget(asim, mesh)->vertices);
    CHECK(qget(asim2, ai)->strategy == "something entirely different but also long");

    CHECK_FALSE(asim2.equals(asim));
    CHECK_THROWS_WITH_AS(asim2.compare(asim),
        "s: compare object of type {'ai', 'stats', 'immaterial', 'mesh'}: 'ai' missing compare",
        object_error);

    CHECK(dom.num_types() == 4);
    dom.garbage_collect_types();
    CHECK(dom.num_types() == 3);
}

TEST_CASE("mutate_to") {
    test_data::lifetimes lstats;
    test_data t;
    domain dom("mt");
    t.register_all_mixins(dom);
    t.create_types(dom);
    t.create_more_types(dom);

    {
        object asim2(dom);
        mutate_to(asim2, *t.t_asim);
        CHECK(&asim2.get_type() == t.t_asim);
    }

    {
        CHECK_THROWS_WITH_AS(object obj(*t.t_ap),
            "mt: mutate to object of type {'actor', 'procedural_geometry'}: 'actor' missing default init",
            object_error);

        auto custom_actor_init = [&](init_new_args args) {
            CHECK(args.mixin_buf);
            CHECK(&args.info == t.actor);
            CHECK(args.target_index == 0);
            new (args.mixin_buf) test_data::m_actor("foo", 10);
        };

        object obj(dom);
        mutate_to(obj, *t.t_ap,
            construct(*t.actor, custom_actor_init));

        CHECK(qget(obj, actor)->name == "foo");
        CHECK(qget(obj, actor)->team == 10);
        CHECK(qget(obj, procedural_geometry)->algo.empty());

        object obj2(dom);
        mutate_to(obj2, *t.t_ap,
            construct(*t.actor, custom_actor_init));
        CHECK(obj.equals(obj2));
        CHECK(obj2.compare(obj) == 0);
        qget(obj, actor)->name = "aaa";
        CHECK_FALSE(obj2.equals(obj));
        CHECK(obj.compare(obj2) < 0);
        qget(obj, actor)->name = "foo";
        CHECK(obj.equals(obj2));

        qget(obj, procedural_geometry)->algo = "bbb";
        CHECK_FALSE(obj.equals(obj2));
        CHECK(obj.compare(obj2) > 0);

        CHECK_THROWS_WITH_AS(auto x = obj.copy(),
            "mt: copy object of type {'actor', 'procedural_geometry'}: 'procedural_geometry' missing copy init",
            object_error);
        CHECK_THROWS_WITH_AS(obj2.copy_from(obj),
            "mt: copy_from object of type {'actor', 'procedural_geometry'}: 'procedural_geometry' missing copy assign",
            object_error);

        object obj3(dom);
        mutate_to(obj3, *t.t_ap
            , construct(*t.actor, custom_actor_init)
            , construct("procedural_geometry", [&](init_new_args args) {
                CHECK(args.mixin_buf);
                CHECK(&args.info == t.procedural_geometry);
                auto pg = new (args.mixin_buf) test_data::m_procedural_geometry;
                pg->algo = "xxx";
            })
        );

        CHECK(qget(obj3, procedural_geometry)->algo == "xxx");

        object acp(dom);
        CHECK_THROWS_WITH_AS(
            mutate_to(acp, *t.t_acp,
                construct(*t.actor, custom_actor_init))
            , "mt: mutate to object of type {'actor', 'controlled', 'physical'}: 'physical' missing default init"
            , object_error);
        CHECK(acp.empty());

        auto custom_physical_init = [&](init_new_args args) {
            CHECK(args.mixin_buf);
            CHECK(&args.info == t.physical);
            new (args.mixin_buf) test_data::m_physical({1, 2, 3});
        };
        mutate_to(acp, *t.t_acp
            , construct("actor", custom_actor_init)
            , construct(*t.physical, custom_physical_init)
        );
        CHECK(qget(acp, actor)->name == "foo");
        CHECK(qget(acp, physical)->data == std::vector<double>{1, 2, 3});

        qget(obj, actor)->name = "error";
        mutate_to(obj, *t.t_acp,
            construct(*t.physical, custom_physical_init));
        CHECK(qget(obj, actor)->name == "error");
        CHECK(qget(obj, physical)->data == std::vector<double>{1, 2, 3});

        CHECK_THROWS_WITH_AS(auto x = obj.copy(), "actor cc error", std::logic_error);
        CHECK_THROWS_WITH_AS(acp.copy_from(obj), "actor asgn error", std::logic_error);
    }
}

TEST_CASE("mutate") {
    test_data::lifetimes lstats;
    test_data::lt_sentry _ls(lstats);
    test_data t;
    domain dom("m");
    t.register_all_mixins(dom);

    auto custom_actor_init = [&](init_new_args args) {
        CHECK(args.mixin_buf);
        CHECK(&args.info == t.actor);
        new (args.mixin_buf) test_data::m_actor("foo", 10);
    };

    {
        object obj(dom);
        mutate(obj, add(*t.movable));
        CHECK(obj.num_mixins() == 1);
        CHECK(obj.has(*t.movable));

        mutate(obj, add(*t.procedural_geometry));
        CHECK(obj.num_mixins() == 2);
        CHECK(obj.has(*t.movable));
        CHECK(obj.has(*t.procedural_geometry));

        mutate(obj,
            dynamix::remove("movable"),
            add("empty")
        );
        CHECK(obj.num_mixins() == 2);
        CHECK(obj.has(*t.empty));
        CHECK(obj.has(*t.procedural_geometry));

        mutate(obj, remove(*t.empty));
        CHECK(obj.num_mixins() == 1);
        CHECK(obj.has(*t.procedural_geometry));
    }

    {
        object obj(dom);

        mutate(obj, add(*t.movable));
        CHECK_THROWS_WITH_AS(mutate(obj, add(*t.actor)),
            "m: mutate to object of type {'movable', 'actor'}: 'actor' missing default init",
            object_error);

        CHECK(obj.num_mixins() == 1);
        CHECK(obj.has(*t.movable));

        mutate(obj, add(*t.actor, custom_actor_init));
        CHECK(obj.num_mixins() == 2);
        CHECK(obj.has(*t.movable));
        CHECK(obj.has(*t.actor));
        qget(obj, actor)->name = "foo";
    }

    {
        object obj(dom);
        mutate(obj).add(*t.movable).add("procedural_geometry");
        auto& t_mp = obj.get_type();
        CHECK(obj.num_mixins() == 2);
        CHECK(obj.has(*t.movable));
        CHECK(t_mp.index_of("movable") == 0);
        CHECK(obj.has(*t.procedural_geometry));

        mutate(obj).remove("movable").add(*t.empty);
        CHECK(obj.num_mixins() == 2);
        CHECK(obj.has(*t.empty));
        CHECK(obj.has(*t.procedural_geometry));
        CHECK(obj.get_type().index_of("empty") == 1);

        mutate(obj).remove(*t.empty);
        CHECK(obj.num_mixins() == 1);
        CHECK(obj.has(*t.procedural_geometry));

        mutate(obj)
            .add(*t.actor)
            .add(*t.movable)
            .remove("actor")
            .to_back(*t.procedural_geometry);
        CHECK(obj.get_type() == t_mp);

        CHECK_THROWS_WITH_AS(mutate(obj).add("actor"),
            "m: mutate to object of type {'movable', 'procedural_geometry', 'actor'}: 'actor' missing default init",
            object_error);
        CHECK(obj.get_type() == t_mp);
    }
}

TEST_CASE("more mutation errors") {
    test_data::lifetimes stats;
    test_data::lt_sentry _ls(stats);

    {
        test_data::lifetimes lstats;
        test_data t;
        domain dom("me");
        t.register_all_mixins(dom);
        t.create_types(dom);

        t.mesh->user_data = 100;

        CHECK_THROWS_WITH_AS(object afmi(*t.t_afmi), "mesh init", std::logic_error);

        CHECK(lstats.ai.living == 0);
        CHECK(lstats.ai.total == 1);
        CHECK(lstats.flyer.living == 0);
        CHECK(lstats.flyer.total == 1);
        CHECK(lstats.mesh.total == 0);
        CHECK(lstats.invisible.total == 0);

        t.mesh->user_data = 200;

        CHECK_THROWS_WITH_AS(object afmi(*t.t_afmi),
            "me: mutate to object of type {'ai', 'flyer', 'mesh', 'invisible'}: deafult init 'mesh' failed with error 1000",
            object_error);

        CHECK(lstats.ai.living == 0);
        CHECK(lstats.ai.total == 2);
        CHECK(lstats.flyer.living == 0);
        CHECK(lstats.flyer.total == 2);
        CHECK(lstats.mesh.total == 0);
        CHECK(lstats.invisible.total == 0);

        t.mesh->user_data = 300;

        object afmi(*t.t_afmi);

        CHECK(lstats.ai.living == 1);
        CHECK(lstats.ai.total == 3);
        CHECK(lstats.flyer.living == 1);
        CHECK(lstats.flyer.total == 3);
        CHECK(lstats.mesh.living == 1);
        CHECK(lstats.mesh.total == 1);
        CHECK(lstats.invisible.living == 1);
        CHECK(lstats.invisible.total == 1);

        CHECK_THROWS_WITH_AS(auto x = afmi.copy(),
            "me: copy object of type {'ai', 'flyer', 'mesh', 'invisible'}: copy init 'mesh' failed with error 10",
            object_error);

        CHECK(lstats.ai.living == 1);
        CHECK(lstats.ai.total == 4);
        CHECK(lstats.flyer.living == 1);
        CHECK(lstats.flyer.total == 4);
        CHECK(lstats.mesh.living == 1);
        CHECK(lstats.mesh.total == 1);
        CHECK(lstats.invisible.living == 1);
        CHECK(lstats.invisible.total == 1);

        qget(afmi, ai)->strategy = "some strat";
        qget(afmi, mesh)->vertices.clear();

        object asim(*t.t_asim);
        CHECK(qget(asim, ai)->strategy == "attack");
        CHECK(qget(asim, mesh)->vertices.size() == 5);

        CHECK_THROWS_WITH_AS(asim.copy_matching_from(afmi),
            "me: copy_matching_from object of type {'ai', 'stats', 'immaterial', 'mesh'}: copy assign 'mesh' failed with error 100",
            object_error);

        CHECK(qget(asim, ai)->strategy == "some strat");
        CHECK(qget(asim, mesh)->vertices.size() == 5);

        CHECK(lstats.ai.living == 2);
        CHECK(lstats.ai.c_asgn == 1);
        CHECK(lstats.ai.total == 5);
        CHECK(lstats.stats.living == 1);
        CHECK(lstats.stats.total == 1);
        CHECK(lstats.flyer.living == 1);
        CHECK(lstats.flyer.total == 4);
        CHECK(lstats.mesh.living == 2);
        CHECK(lstats.mesh.total == 2);
        CHECK(lstats.invisible.living == 1);
        CHECK(lstats.invisible.total == 1);

        qget(afmi, ai)->strategy = "another";
        CHECK_THROWS_WITH_AS(asim.copy_from(afmi),
            "me: copy_from object of type {'ai', 'flyer', 'mesh', 'invisible'}: copy assign 'mesh' failed with error 100",
            object_error);

        CHECK(&asim.get_type() == t.t_asim);

        CHECK(qget(asim, ai)->strategy == "another");
        CHECK(qget(asim, mesh)->vertices.size() == 5);

        CHECK(lstats.ai.living == 2);
        CHECK(lstats.ai.c_asgn == 2);
        CHECK(lstats.ai.total == 5);
        CHECK(lstats.stats.living == 1);
        CHECK(lstats.stats.total == 1);
        CHECK(lstats.flyer.living == 1);
        CHECK(lstats.flyer.total == 5);
        CHECK(lstats.mesh.living == 2);
        CHECK(lstats.mesh.total == 2);
        CHECK(lstats.invisible.living == 1);
        CHECK(lstats.invisible.total == 1);

        CHECK(asim.get_type().num_objects() == 1);
        CHECK(afmi.get_type().num_objects() == 1);

        t.mesh->user_data = 0;
        asim.copy_from(afmi);

        CHECK(t.t_asim->num_objects() == 0);
        CHECK(t.t_afmi->num_objects() == 2);
        CHECK(afmi.get_type().num_objects() == 2);
    }

    {
        test_data::lifetimes lstats;
        test_data t;
        domain dom("me");
        t.register_all_mixins(dom);
        t.create_more_types(dom);

        CHECK_THROWS_WITH_AS(object obj(*t.t_pp),
            "me: mutate to object of type {'physical', 'procedural_geometry'}: 'physical' missing default init",
            object_error);
        CHECK_THROWS_WITH_AS(object obj(*t.t_ap),
            "me: mutate to object of type {'actor', 'procedural_geometry'}: 'actor' missing default init",
            object_error);
    }

    {
        test_data::lifetimes lstats;
        test_data t;
        domain dom;
        t.register_all_mixins(dom);

        const mixin_info* asp[] = {t.ai, t.stats, t.physical};
        auto& tasp = dom.get_type(asp);

        auto create_asp_obj = [&]() {
            object obj(dom);
            mutate_to(obj, tasp, construct(*t.physical, [&](init_new_args args) {
                CHECK(args.mixin_buf);
                CHECK(&args.info == t.physical);
                new (args.mixin_buf) test_data::m_physical({1, 2, 3});
            }));
            return obj;
        };

        CHECK(tasp.num_objects() == 0);

        auto obj = create_asp_obj();
        CHECK(obj.get_type() == tasp);

        CHECK(lstats.ai.living == 1);
        CHECK(lstats.ai.total == 1);
        CHECK(lstats.stats.living == 1);
        CHECK(lstats.stats.total == 1);
        CHECK(lstats.physical.living == 1);
        CHECK(lstats.physical.total == 1);
        CHECK(tasp.num_objects() == 1);

        object obj2 = obj.copy();

        CHECK(lstats.ai.living == 2);
        CHECK(lstats.ai.total == 2);
        CHECK(lstats.stats.living == 2);
        CHECK(lstats.stats.total == 2);
        CHECK(lstats.physical.living == 2);
        CHECK(lstats.physical.total == 2);
        CHECK(tasp.num_objects() == 2);

        object eobj(dom);

        t.ai->user_data = 666;
        CHECK_THROWS_WITH_AS(auto x = obj.copy(), "bad alloc", std::logic_error);
        CHECK_THROWS_WITH_AS(eobj.copy_from(obj), "bad alloc", std::logic_error);
        CHECK(lstats.stats.total == 2);
        CHECK_THROWS_WITH_AS(create_asp_obj(), "bad alloc", std::logic_error);

        CHECK(lstats.ai.living == 2);
        CHECK(lstats.ai.total == 2);
        CHECK(lstats.stats.living == 2);
        CHECK(lstats.stats.total == 2);
        CHECK(lstats.physical.living == 2);
        CHECK(lstats.physical.total == 2);
        CHECK(tasp.num_objects() == 2);
        CHECK(eobj.empty());

        t.ai->user_data = 0;
        t.physical->user_data = 666;
        CHECK_THROWS_WITH_AS(auto x = obj.copy(), "bad alloc", std::logic_error);
        CHECK_THROWS_WITH_AS(eobj.copy_from(obj), "bad alloc", std::logic_error);
        CHECK_THROWS_WITH_AS(create_asp_obj(), "bad alloc", std::logic_error);

        CHECK(lstats.ai.living == 2);
        CHECK(lstats.ai.total == 2);
        CHECK(lstats.stats.living == 2);
        CHECK(lstats.stats.total == 2);
        CHECK(lstats.physical.living == 2);
        CHECK(lstats.physical.total == 2);
        CHECK(tasp.num_objects() == 2);
        CHECK(eobj.empty());

        t.ai->user_data = 666;
        CHECK_NOTHROW(obj2.copy_from(obj));
        CHECK_NOTHROW(obj2.copy_from(obj));

        const mixin_info* afp[] = {t.ai, t.flyer, t.physical};
        auto& tafp = dom.get_type(afp);

        CHECK_NOTHROW(obj.reset_type(tafp));
        CHECK(obj.get_type() == tafp);
    }
}

TEST_CASE("seal") {
    test_data t;
    domain dom("sl");
    t.register_all_mixins(dom);
    t.create_types(dom);

    object sobj(dom);
    mutate(sobj, add(*t.movable));
    sobj.seal();

    qget(sobj, movable)->position = 43;
    auto scopy = sobj.copy();
    CHECK_FALSE(scopy.sealed());

    CHECK_THROWS_WITH_AS(sobj.clear(), "sl: clear sealed object of type {'movable'}", object_error);
    CHECK(sobj.equals(scopy));

    {
        CHECK(t.t_afmi->copyable());
        object oobj(*t.t_afmi);
        CHECK_THROWS_WITH_AS(sobj.copy_from(oobj), "sl: mutate sealed object of type {'movable'}", object_error);
        CHECK(sobj.equals(scopy));

        oobj.copy_from(sobj);
        CHECK_FALSE(oobj.sealed());
        CHECK(sobj.equals(oobj));
    }

    {
        object oobj(*t.t_mov);
        CHECK_FALSE(sobj.equals(oobj));
        CHECK_THROWS_WITH_AS(sobj.copy_from(oobj), "sl: mutate sealed object of type {'movable'}", object_error);
        CHECK(sobj.equals(scopy));
    }

    {
        object oobj(*t.t_mov);
        CHECK_THROWS_WITH_AS(sobj = std::move(oobj), "sl: move assign sealed object of type {'movable'}", object_error);
        CHECK(sobj.equals(scopy));
    }

    CHECK_THROWS_WITH_AS(mutate(sobj, add(*t.mesh)), "sl: mutate sealed object of type {'movable'}", object_error);
    CHECK(sobj.equals(scopy));

    CHECK_THROWS_WITH_AS(sobj.reset_type(*t.t_afmi), "sl: reset_type sealed object of type {'movable'}", object_error);
    CHECK(sobj.equals(scopy));

    {
        object oobj = std::move(sobj);
        CHECK(oobj.equals(scopy));
    }

    object csobj(*t.t_asim, {}, true);
    CHECK(csobj.sealed());
}

TEST_CASE("same type") {
    test_data t;
    domain dom("sl");
    t.register_all_mixins(dom);
    t.create_types(dom);

    object obj(*t.t_afmi);
    const auto inv = obj.get(*t.invisible);
    CHECK(inv);

    obj.reset_type(*t.t_afmi);
    CHECK(inv == obj.get(*t.invisible));

    mutate_to(obj, *t.t_afmi);
    CHECK(inv == obj.get(*t.invisible));

    perform_object_mutation(obj, *t.t_afmi);
    CHECK(inv == obj.get(*t.invisible));
}
