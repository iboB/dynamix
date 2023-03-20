// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>
#include <dnmx/type.h>
#include <dnmx/type_mutation.h>
#include <dnmx/mutation_rule_info.h>

#include "s-unity.h"

void setUp(void) {}
void tearDown(void) {}

void empty(void) {
    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);

    dnmx_type_handle type = dnmx_get_empty_type(dom);
    T_NOT_NULL(type);

    CHECK(dnmx_type_num_objects(type) == 0);
    CHECK(dnmx_type_num_mixins(type) == 0);

    dnmx_mixin_info em = dnmx_make_mixin_info();
    CHECK_FALSE(dnmx_type_has(type, &em));
    CHECK_FALSE(dnmx_type_has_by_name(type, dnmx_make_sv_lit("bar")));

    dnmx_feature_info ef = dnmx_make_feature_info();
    CHECK_FALSE(dnmx_type_implements_strong(type, &ef));
    CHECK_FALSE(dnmx_type_implements_strong_by_name(type, dnmx_make_sv_lit("foo")));

    CHECK_FALSE(dnmx_type_implements(type, &ef));

    dnmx_feature_info di = dnmx_make_feature_info();
    di.name = dnmx_make_sv_lit("default");
    di.default_payload = &di;
    CHECK(dnmx_type_implements(type, &di));

    //dnmx_mixin_index_t dnmx_type_is_of();

    CHECK(dnmx_type_is_default_constructible(type));
    CHECK(dnmx_type_is_copy_constructible(type));
    CHECK(dnmx_type_is_copy_assignable(type));
    CHECK(dnmx_type_is_copyable(type));
    CHECK(dnmx_type_is_equality_comparable(type));
    CHECK(dnmx_type_is_comparable(type));

    dnmx_destroy_domain(dom);
}

void simple(void) {
    dnmx_feature_info
        run = dnmx_make_feature_info(),
        shoot = dnmx_make_feature_info(),
        jump = dnmx_make_feature_info();

    run.name = dnmx_make_sv_lit("run");
    run.allow_clashes = true;
    shoot.name = dnmx_make_sv_lit("shoot");
    jump.name = dnmx_make_sv_lit("jump");

    dnmx_mixin_info
        athlete = dnmx_make_mixin_info(),
        warrior = dnmx_make_mixin_info(),
        shooter = dnmx_make_mixin_info();

    athlete.name = dnmx_make_sv_lit("athlete");
    dnmx_feature_for_mixin ath_skills[] = {{&run}, {&jump}};
    athlete.features = ath_skills;
    athlete.num_features = 2;

    warrior.name = dnmx_make_sv_lit("warrior");
    dnmx_feature_for_mixin war_skills[] = {{&run}, {&shoot}};
    warrior.features = war_skills;
    warrior.num_features = 2;

    shooter.name = dnmx_make_sv_lit("shooter");
    dnmx_feature_for_mixin sho_skills[] = {{&shoot}};
    shooter.features = sho_skills;
    shooter.num_features = 1;

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);
    dnmx_register_mixin(dom, &athlete);
    dnmx_register_mixin(dom, &warrior);
    dnmx_register_mixin(dom, &shooter);

    const dnmx_mixin_info* ar_aw[] = {&athlete, &warrior};
    const dnmx_mixin_info* ar_as[] = {&athlete, &shooter};
    const dnmx_mixin_info* ar_ws[] = {&warrior, &shooter};

    CHECK(dnmx_get_num_types(dom) == 0);
    dnmx_type_handle taw = dnmx_get_type_from_infos(dom, ar_aw, 2);
    dnmx_type_handle type = taw;
    T_NOT_NULL(type);
    CHECK(dnmx_get_num_types(dom) == 1);

    CHECK(dnmx_type_num_objects(type) == 0);
    CHECK(dnmx_type_num_mixins(type) == 2);

    CHECK(dnmx_type_has(type, &athlete));
    CHECK(dnmx_type_has_by_name(type, dnmx_make_sv_lit("warrior")));
    CHECK_FALSE(dnmx_type_has(type, &shooter));
    CHECK_FALSE(dnmx_type_has_by_name(type, dnmx_make_sv_lit("shooter")));

    CHECK(dnmx_type_implements_strong(type, &run));
    CHECK(dnmx_type_implements_strong_by_name(type, dnmx_make_sv_lit("shoot")));
    CHECK(dnmx_type_implements(type, &jump));

    CHECK_FALSE(dnmx_type_is_default_constructible(type));
    CHECK_FALSE(dnmx_type_is_copy_constructible(type));
    CHECK_FALSE(dnmx_type_is_copy_assignable(type));
    CHECK_FALSE(dnmx_type_is_copyable(type));
    CHECK_FALSE(dnmx_type_is_equality_comparable(type));
    CHECK_FALSE(dnmx_type_is_comparable(type));

    {
        dnmx_ftable_entry fe = dnmx_ftable_at(type, shoot.id);
        CHECK(fe.end - fe.begin == 1);
        fe = dnmx_ftable_at(type, run.id);
        CHECK(fe.end - fe.begin == 2);
    }

    dnmx_type_handle tas = dnmx_get_type_from_infos(dom, ar_as, 2);
    T_NOT_NULL(tas);
    CHECK(tas != taw);
    dnmx_type_handle tws = dnmx_get_type_from_infos(dom, ar_ws, 2);
    T_NULL(tws);

    type = dnmx_get_type_from_infos(dom, ar_aw, 2);
    CHECK(type == taw);

    CHECK(dnmx_get_num_types(dom) == 2);

    type = dnmx_get_type_from_infos(dom, ar_aw, 1);
    CHECK(type != taw);
    {
        dnmx_ftable_entry fe = dnmx_ftable_at(type, shoot.id);
        CHECK(fe.end - fe.begin == 0);
        fe = dnmx_ftable_at(type, run.id);
        CHECK(fe.end - fe.begin == 1);
    }

    dnmx_destroy_domain(dom);
}

void mutations(void) {
    dnmx_feature_info
        run = dnmx_make_feature_info(),
        shoot = dnmx_make_feature_info(),
        jump = dnmx_make_feature_info();

    run.name = dnmx_make_sv_lit("run");
    run.allow_clashes = true;
    shoot.name = dnmx_make_sv_lit("shoot");
    jump.name = dnmx_make_sv_lit("jump");

    dnmx_mixin_info
        athlete = dnmx_make_mixin_info(),
        warrior = dnmx_make_mixin_info(),
        shooter = dnmx_make_mixin_info();

    athlete.name = dnmx_make_sv_lit("athlete");
    dnmx_feature_for_mixin ath_skills[] = {{&run}, {&jump}};
    athlete.features = ath_skills;
    athlete.num_features = 2;

    warrior.name = dnmx_make_sv_lit("warrior");
    dnmx_feature_for_mixin war_skills[] = {{&run}, {&shoot}};
    warrior.features = war_skills;
    warrior.num_features = 2;

    shooter.name = dnmx_make_sv_lit("shooter");
    dnmx_feature_for_mixin sho_skills[] = {{&shoot}};
    shooter.features = sho_skills;
    shooter.num_features = 1;

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings) { 0 }, 0, NULL);
    dnmx_register_mixin(dom, &athlete);
    dnmx_register_mixin(dom, &warrior);
    dnmx_register_mixin(dom, &shooter);

    {
        dnmx_type_mutation_handle mut = dnmx_create_type_mutation_empty(dom);
        CHECK(dnmx_type_mutation_is_noop(mut));
        CHECK(dnmx_type_mutation_get_base(mut) == dnmx_get_empty_type(dom));
        dnmx_destroy_unused_type_mutation(mut);
    }

    dnmx_type_handle t_aw;
    {
        dnmx_type_mutation_handle mut = dnmx_create_type_mutation_empty(dom);
        dnmx_type_template_handle new_type = dnmx_type_mutation_get_new_type(mut);

        CHECK(dnmx_type_template_add_if_lacking(new_type, &warrior));
        CHECK(dnmx_type_template_add(new_type, &athlete));
        CHECK_FALSE(dnmx_type_template_add_if_lacking(new_type, &warrior));
        CHECK(&warrior == dnmx_type_template_to_back_by_name(new_type, dnmx_make_sv_lit("warrior")));

        CHECK(dnmx_type_template_has(new_type, &warrior));
        CHECK(&athlete == dnmx_type_template_has_by_name(new_type, dnmx_make_sv_lit("athlete")));
        CHECK(dnmx_type_template_implements_strong(new_type, &shoot));
        CHECK(&run == dnmx_type_template_implements_strong_by_name(new_type, dnmx_make_sv_lit("run")));

        dnmx_mixin_index_t num;
        const dnmx_mixin_info* const* infos = dnmx_type_template_get_mixins(new_type, &num);
        CHECK(num == 2);
        CHECK(infos[0] == &athlete);
        CHECK(infos[1] == &warrior);

        CHECK(dnmx_type_mutation_is_adding_mixins(mut));
        CHECK_FALSE(dnmx_type_mutation_is_removing_mixins(mut));
        CHECK(&athlete == dnmx_type_mutation_is_adding_by_name(mut, dnmx_make_sv_lit("athlete")));
        CHECK(dnmx_type_mutation_is_adding(mut, &warrior));
        CHECK_FALSE(dnmx_type_mutation_is_adding(mut, &shooter));

        t_aw = dnmx_get_type(dom, &mut);
        CHECK_FALSE(mut);
        CHECK(t_aw);
    }

    dnmx_type_handle t_as;
    {
        dnmx_type_mutation_handle mut = dnmx_create_type_mutation_from_type(t_aw);
        dnmx_type_template_handle new_type = dnmx_type_mutation_get_new_type(mut);
        CHECK_FALSE(dnmx_type_template_remove_by_name(new_type, dnmx_make_sv_lit("shooter")));
        CHECK_FALSE(dnmx_type_template_remove(new_type, &shooter));
        CHECK(&warrior == dnmx_type_template_remove_by_name(new_type, dnmx_make_sv_lit("warrior")));
        CHECK(dnmx_type_template_add(new_type, &shooter));
        CHECK(dnmx_type_mutation_is_removing_mixins(mut));
        CHECK(dnmx_type_mutation_is_removing(mut, &warrior));
        CHECK(&warrior == dnmx_type_mutation_is_removing_by_name(mut, dnmx_make_sv_lit("warrior")));
        CHECK(dnmx_type_mutation_get_base(mut) == t_aw);
        t_as = dnmx_get_type(dom, &mut);
    }

    {
        dnmx_type_mutation_handle mut = dnmx_create_type_mutation_empty(dom);
        const dnmx_mixin_info* ar_aw[] = {&athlete, &warrior};
        dnmx_type_template_handle new_type = dnmx_type_mutation_get_new_type(mut);
        dnmx_type_template_set_mixins(new_type, ar_aw, 2);
        CHECK(t_aw == dnmx_get_type(dom, &mut));
    }

    {
        const dnmx_mixin_info* ar_as[] = {&athlete, &shooter};
        CHECK(t_as == dnmx_get_type_from_infos(dom, ar_as, 2));
    }

    CHECK(dnmx_get_num_types(dom) == 2);
    dnmx_destroy_domain(dom);
}

typedef struct mr_dep {
    dnmx_mixin_info* primary;
    dnmx_mixin_info* dependent;
} mr_dep;
dnmx_error_return_t apply_dep(dnmx_type_mutation_handle mut, uintptr_t user_data) {
    mr_dep* dep = (mr_dep*)(user_data);
    dnmx_type_template_handle new_type = dnmx_type_mutation_get_new_type(mut);
    if (dnmx_type_template_has(new_type, dep->primary)) {
        dnmx_type_template_add_if_lacking(new_type, dep->dependent);
    }
    return dnmx_result_success;
}


void mutation_rules(void) {
    dnmx_mixin_info
        athlete = dnmx_make_mixin_info(),
        warrior = dnmx_make_mixin_info();

    athlete.name = dnmx_make_sv_lit("athlete");
    warrior.name = dnmx_make_sv_lit("warrior");
    warrior.dependency = true;

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings) { 0 }, 0, NULL);
    dnmx_register_mixin(dom, &athlete);
    dnmx_register_mixin(dom, &warrior);

    mr_dep dep = {&athlete, &warrior};
    dnmx_mutation_rule_info rule = {0};
    rule.user_data = (uintptr_t)&dep;
    rule.apply = apply_dep;
    dnmx_add_mutation_rule(dom, &rule);

    const dnmx_mixin_info* ar_a[] = {&athlete};

    {
        dnmx_type_handle type = dnmx_get_type_from_infos(dom, ar_a, 1);
        CHECK(type);
        CHECK(dnmx_type_num_mixins(type) == 2);
        CHECK(dnmx_type_has(type, &athlete));
        CHECK(dnmx_type_has(type, &warrior));
    }

    dnmx_remove_mutation_rule(dom, &rule);

    {
        dnmx_type_handle type = dnmx_get_type_from_infos(dom, ar_a, 1);
        CHECK(type);
        CHECK(dnmx_type_num_mixins(type) == 1);
        CHECK(dnmx_type_has(type, &athlete));
        CHECK_FALSE(dnmx_type_has(type, &warrior));
    }

    dnmx_destroy_domain(dom);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(empty);
    RUN_TEST(simple);
    RUN_TEST(mutations);
    RUN_TEST(mutation_rules);
    return UNITY_END();
}
