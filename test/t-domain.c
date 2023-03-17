// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>

#include "s-unity.h"

void setUp(void) {}
void tearDown(void) {}

void empty(void) {
    int c1 = 0, c2 = 0;
    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 27, &c1);
    T_SV_EXPECT("test", dnmx_get_domain_name(dom));
    dnmx_domain_settings s = dnmx_get_domain_settings(dom);
    CHECK_FALSE(s.canonicalize_types);
    CHECK_FALSE(s.allow_duplicate_feature_names);
    CHECK_FALSE(s.allow_duplicate_mixin_names);
    CHECK(dnmx_get_domain_user_data(dom) == 27);
    CHECK(dnmx_get_domain_context(dom) == &c1);

    dnmx_set_domain_user_data(dom, 34);
    CHECK(dnmx_get_domain_user_data(dom) == 34);

    dnmx_set_domain_context(dom, &c2);
    CHECK(dnmx_get_domain_context(dom) == &c2);

    CHECK(dnmx_get_num_types(dom) == 0);

    dnmx_destroy_domain(dom);
}

void feature(void) {
    dnmx_feature_info
        run = dnmx_make_feature_info(),
        shoot = dnmx_make_feature_info(),
        jump = dnmx_make_feature_info(),
        jump2 = dnmx_make_feature_info();

    run.name = dnmx_make_sv_lit("name");
    shoot.name = dnmx_make_sv_lit("shoot");
    jump.name = dnmx_make_sv_lit("jump");
    jump2.name = dnmx_make_sv_lit("jump");

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);

    T_SUCCESS(dnmx_register_feature(dom, &run));
    T_SUCCESS(dnmx_register_feature(dom, &shoot));
    T_SUCCESS(dnmx_register_feature(dom, &jump));

    T_FAIL(dnmx_register_feature(dom, &jump2));
    dnmx_unregister_feature(dom, &jump);
    T_SUCCESS(dnmx_register_feature(dom, &jump2));

    CHECK(dnmx_get_feature_info_by_id(dom, (dnmx_feature_id) { 0 }) == &run);
    CHECK(dnmx_get_feature_info_by_name(dom, dnmx_make_sv_lit("jump")) == &jump2);

    dnmx_unregister_feature(dom, &jump2);
    T_NULL(dnmx_get_feature_info_by_name(dom, dnmx_make_sv_lit("jump")));

    dnmx_destroy_domain(dom);
}

void mixin(void) {
    dnmx_feature_info
        run = dnmx_make_feature_info(),
        shoot = dnmx_make_feature_info(),
        jump = dnmx_make_feature_info();

    run.name = dnmx_make_sv_lit("name");
    shoot.name = dnmx_make_sv_lit("shoot");
    jump.name = dnmx_make_sv_lit("jump");

    dnmx_mixin_info
        athlete = dnmx_make_mixin_info(),
        warrior = dnmx_make_mixin_info(),
        prewar = dnmx_make_mixin_info();

    athlete.name = dnmx_make_sv_lit("athlete");
    dnmx_feature_for_mixin ath_skills[] = {{&run}, {&jump}};
    athlete.features = ath_skills;
    athlete.num_features = 2;

    warrior.name = dnmx_make_sv_lit("warrior");
    dnmx_feature_for_mixin war_skills[] = {{&run}, {&shoot}};
    warrior.features = war_skills;
    warrior.num_features = 2;

    prewar.name = dnmx_make_sv_lit("warrior");

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);

    T_SUCCESS(dnmx_register_mixin(dom, &athlete));
    CHECK(dnmx_get_feature_info_by_name(dom, dnmx_make_sv_lit("jump")) == &jump);
    T_NULL(dnmx_get_feature_info_by_name(dom, dnmx_make_sv_lit("shoot")));

    T_SUCCESS(dnmx_register_mixin(dom, &prewar));

    T_FAIL(dnmx_register_mixin(dom, &warrior));
    dnmx_unregister_mixin(dom, &prewar);

    T_SUCCESS(dnmx_register_mixin(dom, &warrior));
    CHECK(dnmx_get_feature_info_by_name(dom, dnmx_make_sv_lit("shoot")) == &shoot);

    dnmx_destroy_domain(dom);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(empty);
    RUN_TEST(feature);
    RUN_TEST(mixin);
    return UNITY_END();
}
