// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <doctest/doctest.h>

#include <dynamix/domain.hpp>
#include <dynamix/mutation_rule_info.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/type.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/type_mutation.hpp>

#include <itlib/qalgorithm.hpp>
#include <itlib/strutil.hpp>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("domain add/remove rules") {
    auto noop = [](dnmx_type_mutation_handle, uintptr_t) { return dnmx_result_success; };

    {
        domain dom;
        mutation_rule_info mr = {};
        dom.remove_mutation_rule(mr); // should be safe
        CHECK_THROWS_WITH_AS(dom.add_mutation_rule(mr), "bad mutation rule", domain_error);
        mr.apply = noop;
        CHECK_NOTHROW(dom.add_mutation_rule(mr));

        test_data t;
        t.register_all_mixins(dom);
        t.create_types(dom);

        dom.remove_mutation_rule(mr);
        dom.remove_mutation_rule(mr); // should be safe
    }

    {
        mutation_rule_info mr1 = {};
        mutation_rule_info mr2 = {};
        mutation_rule_info mr3 = {};

        domain dom;
        CHECK_THROWS_WITH_AS(dom.add_mutation_rule(mr1), "bad mutation rule", domain_error);
        mr1.apply = noop;
        mr2.apply = noop;
        mr3.apply = noop;
        CHECK(dom.num_mutation_rules() == 0);
        dom.add_mutation_rule(mr1);
        dom.add_mutation_rule(mr2);
        dom.add_mutation_rule(mr3);
        CHECK(dom.num_mutation_rules() == 3);

        dom.add_mutation_rule(mr3);
        CHECK(dom.num_mutation_rules() == 3); // ref count

        mutation_rule_info mr_ext = {{}, noop, 0, 0};
        dom.add_mutation_rule(mr_ext);
        CHECK(dom.num_mutation_rules() == 4);

        dom.remove_mutation_rule(mr1);
        dom.remove_mutation_rule(mr2);
        dom.remove_mutation_rule(mr3);
        CHECK(dom.num_mutation_rules() == 2); // mr3, mr_ext

        dom.remove_mutation_rule(mr2);
        CHECK(dom.num_mutation_rules() == 2); // mr3, mr_ext

        dom.remove_mutation_rule(mr3);
        CHECK(dom.num_mutation_rules() == 1); // mr_ext
    }
}

struct mut_rule_test_data {
    test_data t;
    std::vector<int> log;

    void check_log(std::vector<int> expected) {
        CHECK(log == expected);
        log.clear();
    };
};

TEST_CASE("apply rules") {
    mut_rule_test_data mrtd;
    domain dom;

    mrtd.t.register_all_mixins(dom);
    CHECK(dom.num_mutation_rules() == 0);

    util::mixin_info_data to_actor_mesh;
    {
        auto add_to_actor_mesh = [](dnmx_type_mutation_handle mutation, uintptr_t ud) {
            CHECK(ud);
            auto mrtd = reinterpret_cast<mut_rule_test_data*>(ud);
            mrtd->log.push_back(1);
            auto& td = mrtd->t;
            if (td.mesh->user_data == 111) return -1;
            auto mut = type_mutation::from_c_handle(mutation);
            auto& nt = mut->new_type();
            if (nt.has(*td.actor) || nt.has(*td.mesh)) {
                mut->add_if_lacking("tam");
            }
            return result_success;
        };
        util::mixin_info_data_builder builder(to_actor_mesh, "tam");
        builder.adds_mutation_rule(add_to_actor_mesh, reinterpret_cast<uintptr_t>(&mrtd));
        to_actor_mesh.register_in(dom);
    }
    CHECK(dom.num_mutation_rules() == 1);

    util::mixin_info_data wheeled;
    {
        auto add_movable_to_wheeled = [](dnmx_type_mutation_handle mutation, uintptr_t ud) {
            CHECK(ud);
            auto mrtd = reinterpret_cast<mut_rule_test_data*>(ud);
            auto& td = mrtd->t;
            if (td.movable->user_data == 666) throw std::logic_error("too many wheels");
            mrtd->log.push_back(2);
            auto mut = type_mutation::from_c_handle(mutation);
            if (mut->adding("wheeled") && mut->new_type().lacks(*td.movable)) {
                mut->add(*td.movable);
            }
            else if (mut->removing("wheeled")) {
                mut->remove(*td.movable);
            }
            return result_success;
        };
        util::mixin_info_data_builder builder(wheeled, "wheeled");
        using namespace util::builder_literals;
        builder.adds_mutation_rule(add_movable_to_wheeled, reinterpret_cast<uintptr_t>(&mrtd), 100_prio);
        builder.dependency(false);
        wheeled.register_in(dom);
    }
    CHECK(dom.num_mutation_rules() == 2);

    auto ban_empty = [](dnmx_type_mutation_handle mutation, uintptr_t ud) {
        auto mrtd = reinterpret_cast<mut_rule_test_data*>(ud);
        mrtd->log.push_back(0);
        auto mut = type_mutation::from_c_handle(mutation);
        itlib::erase_all_if(mut->mod_new_type().mixins, [](const mixin_info* info) {
            return itlib::starts_with(info->name.to_std(), "empty");
            });
        return result_success;
    };
    mutation_rule_info mri;
    mri.name = dnmx_make_sv_lit("mri");
    mri.apply = ban_empty;
    mri.user_data = reinterpret_cast<uintptr_t>(&mrtd);
    mri.order_priority = -56;
    dom.add_mutation_rule(mri);
    CHECK(dom.num_mutation_rules() == 3);

    auto& t = mrtd.t;

    {
        const mixin_info* mixins[] = {t.physical, t.ai};
        auto& type = dom.get_type(mixins);
        CHECK(type.num_mixins() == 2);
        CHECK(type.has(*t.physical));
        CHECK(type.has(*t.ai));
        mrtd.check_log({0, 1, 2});
        CHECK(dom.num_type_queries() == 1);
    }

    {
        const mixin_info* mixins[] = {t.actor, t.procedural_geometry};
        auto& type = dom.get_type(mixins);
        CHECK(type.num_mixins() == 3);
        CHECK(type.index_of(t.actor->id) == 0);
        CHECK(type.index_of(t.procedural_geometry->id) == 1);
        CHECK(type.index_of(to_actor_mesh.info.id) == 2);
        mrtd.check_log({0, 1, 2, 0, 1, 2});
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 2);

        auto& type_b = dom.get_type(mixins);
        CHECK(&type == &type_b);
        mrtd.check_log({}); // no rules applied for stored query

        const mixin_info* mixins2[] = {t.actor, t.procedural_geometry, &to_actor_mesh.info};
        auto& type2 = dom.get_type(mixins2);
        CHECK(&type == &type2);
        mrtd.check_log({0, 1, 2});
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 3);

        const mixin_info* mixins3[] = {&to_actor_mesh.info};
        CHECK(&dom.get_type(mixins3) == &dom.get_empty_type());
        mrtd.check_log({0, 1, 2, 0, 1, 2});
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 4);
    }

    {
        t.mesh->user_data = 111;
        const mixin_info* mixins[] = {t.mesh};
        CHECK_THROWS_WITH_AS(dom.get_type(mixins), "mutation rule", mutation_user_error);
        mrtd.check_log({0, 1});
        t.mesh->user_data = 0;
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 4);
    }

    {
        t.movable->user_data = 666;
        const mixin_info* mixins[] = {t.movable};
        CHECK_THROWS_WITH_AS(dom.get_type(mixins), "too many wheels", std::logic_error);
        mrtd.check_log({0, 1});
        t.movable->user_data = 0;
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 4);
    }

    {
        const mixin_info* mixins[] = {&wheeled.info, t.empty_with_features, t.mesh, t.empty};
        auto& type = dom.get_type(mixins);
        mrtd.check_log({0, 1, 2, 0, 1, 2});
        CHECK(type.num_mixins() == 4);
        CHECK(type.mixins[0] == &wheeled.info);
        CHECK(type.mixins[1] == t.mesh);
        CHECK(type.mixins[2] == &to_actor_mesh.info);
        CHECK(type.mixins[3] == t.movable);
        CHECK(dom.num_types() == 3);
        CHECK(dom.num_type_queries() == 5);
    }

    to_actor_mesh.unregister_from(dom);

    {
        const mixin_info* mixins[] = {t.actor, t.procedural_geometry};
        auto& type = dom.get_type(mixins);
        mrtd.check_log({0, 2});
        CHECK(type.num_mixins() == 2);
        CHECK(type.index_of(t.actor->id) == 0);
        CHECK(type.index_of(t.procedural_geometry->id) == 1);
        CHECK(dom.num_types() == 2);
        CHECK(dom.num_type_queries() == 1);
    }

    {
        const mixin_info* mixins[] = {&wheeled.info, t.movable};
        auto& type = dom.get_type(mixins);
        mrtd.check_log({0, 2});
        CHECK(type.num_mixins() == 2);
        CHECK(type.mixins[0] == &wheeled.info);
        CHECK(type.mixins[1] == t.movable);
        CHECK(dom.num_types() == 3);
        CHECK(dom.num_type_queries() == 2);
    }

    {
        const mixin_info* mixins[] = {&wheeled.info};
        auto& type = dom.get_type(mixins);
        mrtd.check_log({0, 2, 0, 2});
        CHECK(type.num_mixins() == 2);
        CHECK(type.mixins[0] == &wheeled.info);
        CHECK(type.mixins[1] == t.movable);
        CHECK(dom.num_types() == 3);
        CHECK(dom.num_type_queries() == 3);
    }
}

TEST_CASE("rule interdependency") {
    mut_rule_test_data mrtd;
    domain dom;
    mrtd.t.register_all_mixins(dom);

    util::mixin_info_data wheeled;
    {
        auto add_movable_to_wheeled = [](dnmx_type_mutation_handle mutation, uintptr_t ud) {
            auto mrtd = reinterpret_cast<mut_rule_test_data*>(ud);
            mrtd->log.push_back(1);
            auto& td = mrtd->t;
            auto mut = type_mutation::from_c_handle(mutation);
            if (mut->adding("wheeled")) {
                mut->add_if_lacking(*td.movable);
            }
            else if (mut->removing("wheeled")) {
                mut->remove(*td.movable);
            }
            return result_success;
        };
        util::mixin_info_data_builder builder(wheeled, "wheeled");
        using namespace util::builder_literals;
        builder.adds_mutation_rule(add_movable_to_wheeled, reinterpret_cast<uintptr_t>(&mrtd), 100_prio); // last
        builder.dependency(false);
        wheeled.register_in(dom);
    }

    util::mixin_info_data tracker;
    {
        auto add_tracker_to_movable = [](dnmx_type_mutation_handle mutation, uintptr_t ud) {
            auto mrtd = reinterpret_cast<mut_rule_test_data*>(ud);
            mrtd->log.push_back(2);
            auto& td = mrtd->t;
            auto mut = type_mutation::from_c_handle(mutation);
            if (mut->new_type().has(*td.movable)) {
                mut->add_if_lacking("tracker");
            }
            return result_success;
        };
        util::mixin_info_data_builder builder(tracker, "tracker");
        using namespace util::builder_literals;
        builder.adds_mutation_rule(add_tracker_to_movable, reinterpret_cast<uintptr_t>(&mrtd), -5_prio); // first
        tracker.register_in(dom);
    }

    auto& t = mrtd.t;

    {
        const mixin_info* mixins[] = {&wheeled.info};
        auto& type = dom.get_type(mixins);
        mrtd.check_log({2, 1, 2, 1, 2, 1});
        CHECK(type.num_mixins() == 3);
        CHECK(type.mixins[0] == &wheeled.info);
        CHECK(type.mixins[1] == t.movable);
        CHECK(type.mixins[2] == &tracker.info);
        CHECK(dom.num_types() == 1);
        CHECK(dom.num_type_queries() == 1);
    }

    // introduce cyclic dep
    mutation_rule_info mri = {};
    mri.name = dnmx_make_sv_lit("mri");
    mri.apply = [](dnmx_type_mutation_handle mutation, uintptr_t) {
        type_mutation::from_c_handle(mutation)->add("actor");
        return result_success;
    };
    dom.add_mutation_rule(mri);

    {
        const mixin_info* mixins[] = {&wheeled.info};
        CHECK_THROWS_WITH_AS(dom.get_type(mixins), "rule interdependency too deep or cyclic", domain_error);
        CHECK(dom.num_types() == 1);
        CHECK(dom.num_type_queries() == 0);
    }
}