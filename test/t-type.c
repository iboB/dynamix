// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>
#include <dnmx/type.h>

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

    run.name = dnmx_make_sv_lit("name");
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
    dnmx_type_handle taw = dnmx_get_type(dom, ar_aw, 2);
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

    dnmx_type_handle tas = dnmx_get_type(dom, ar_as, 2);
    T_NOT_NULL(tas);
    CHECK(tas != taw);
    dnmx_type_handle tws = dnmx_get_type(dom, ar_ws, 2);
    T_NULL(tws);

    type = dnmx_get_type(dom, ar_aw, 2);
    CHECK(type == taw);

    CHECK(dnmx_get_num_types(dom) == 2);

    type = dnmx_get_type(dom, ar_aw, 1);
    CHECK(type != taw);
    {
        dnmx_ftable_entry fe = dnmx_ftable_at(type, shoot.id);
        CHECK(fe.end - fe.begin == 0);
        fe = dnmx_ftable_at(type, run.id);
        CHECK(fe.end - fe.begin == 1);
    }

    dnmx_destroy_domain(dom);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(empty);
    RUN_TEST(simple);
    return UNITY_END();
}
