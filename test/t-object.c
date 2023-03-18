// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>
#include <dnmx/type.h>
#include <dnmx/object.h>
#include <dnmx/mixin_info_util.h>

#include "s-unity.h"

void setUp(void) {}
void tearDown(void) {}

void empty(void) {
    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);
    dnmx_object_handle obj = dnmx_create_object_empty(dom);
    T_NOT_NULL(obj);
    CHECK(dnmx_object_get_domain(obj) == dom);
    CHECK(dnmx_object_get_type(obj) == dnmx_get_empty_type(dom));
    CHECK(dnmx_object_num_mixins(obj) == 0);
    CHECK(dnmx_object_is_empty(obj));
    CHECK_FALSE(dnmx_object_is_sealed(obj));

    dnmx_mixin_info empty = dnmx_make_mixin_info();
    T_NULL(dnmx_object_get(obj, &empty));
    T_NULL(dnmx_object_get_by_name(obj, dnmx_make_sv_lit("foo")));
    T_NULL(dnmx_object_get_at(obj, 57));
    T_NULL(dnmx_object_get_mut(obj, &empty));
    T_NULL(dnmx_object_get_mut_by_name(obj, dnmx_make_sv_lit("bar")));
    T_NULL(dnmx_object_get_mut_at(obj, 0));

    dnmx_object_clear(obj);
    CHECK(dnmx_object_get_type(obj) == dnmx_get_empty_type(dom));
    dnmx_object_reset_type(obj, NULL);
    CHECK(dnmx_object_get_type(obj) == dnmx_get_empty_type(dom));

    {
        dnmx_object_handle obj2 = dnmx_create_object_empty(dom);
        T_NOT_NULL(obj2);
        CHECK(dnmx_object_get_type(obj2) == dnmx_get_empty_type(dom));
        CHECK(dnmx_object_is_empty(obj2));

        CHECK(dnmx_object_equals(obj, obj2));

        dnmx_int_result cmp_res = dnmx_object_compare(obj2, obj);
        CHECK(cmp_res.error == dnmx_result_success);
        CHECK(cmp_res.result == 0);

        CHECK(dnmx_object_copy_from(obj2, obj) == dnmx_result_success);
        CHECK(dnmx_object_get_type(obj2) == dnmx_get_empty_type(dom));
        CHECK(dnmx_object_is_empty(obj2));

        dnmx_destroy_object(obj2);
    }

    {
        dnmx_object_handle obj2 = dnmx_create_object_default(dnmx_get_empty_type(dom));
        CHECK(dnmx_object_is_empty(obj2));
        CHECK(dnmx_object_get_type(obj2) == dnmx_get_empty_type(dom));
        dnmx_destroy_object(obj2);
    }

    {
        dnmx_object_handle obj2 = dnmx_create_object_copy(obj);
        CHECK(dnmx_object_is_empty(obj2));
        CHECK(dnmx_object_get_type(obj2) == dnmx_get_empty_type(dom));
        dnmx_destroy_object(obj2);
    }

    {
        dnmx_object_handle obj2 = dnmx_create_object_usurp(obj);
        CHECK(dnmx_object_is_empty(obj2));
        CHECK(dnmx_object_get_type(obj2) == dnmx_get_empty_type(dom));
        dnmx_destroy_object(obj2);
    }

    dnmx_destroy_object(obj);
    dnmx_destroy_domain(dom);
}

typedef struct warrior {
    float target_x, target_y;
    float speed;
} warrior;

typedef struct athlete {
    int speed;
    dnmx_sv target_name;
} athlete;

typedef struct shooter {
    double target_x, target_y;
} shooter;

typedef struct jumper {
    float height;
} jumper;

typedef struct mi_ex {
    dnmx_mixin_info info;
    dnmx_feature_for_mixin ffm[2];
    int skills[2];
} mi_ex;

dnmx_error_return_t init_athlete(const dnmx_mixin_info* info, void* m) {
    athlete* sh = (athlete*)m;
    memset(sh, 0, sizeof(athlete)); // fill padding
    sh->speed = 10;
    sh->target_name = dnmx_make_sv_lit("pernik");
    if (info->user_data == 666) return -1;
    return dnmx_result_success;
}

#if !defined(_countof)
#   define _countof(a) (sizeof(a)/sizeof(*(a)))
#endif

void simple(void) {
    dnmx_feature_info
        run = dnmx_make_feature_info(),
        shoot = dnmx_make_feature_info(),
        jump = dnmx_make_feature_info();
    run.name = dnmx_make_sv_lit("run");
    run.allow_clashes = true;
    shoot.name = dnmx_make_sv_lit("shoot");
    jump.name = dnmx_make_sv_lit("jump");

    mi_ex mi_warrior = {0}, mi_athlete = {0}, mi_shooter = {0}, mi_jumper = {0};

    DNMX_COMMON_INIT_MI(mi_warrior.info, warrior);
    mi_warrior.skills[0] = 42;
    mi_warrior.skills[1] = 43;
    mi_warrior.ffm[0] = (dnmx_feature_for_mixin){&shoot, mi_warrior.skills, 0, 0};
    mi_warrior.ffm[1] = (dnmx_feature_for_mixin){&run, mi_warrior.skills + 1, 0, 0};
    mi_warrior.info.compare = dnmx_mixin_common_cmp_func;
    mi_warrior.info.features = mi_warrior.ffm;
    mi_warrior.info.num_features = 2;

    DNMX_COMMON_INIT_MI(mi_athlete.info, athlete);
    mi_athlete.skills[0] = 69;
    mi_athlete.skills[1] = 70;
    mi_athlete.ffm[0] = (dnmx_feature_for_mixin){&jump, mi_athlete.skills, 0, 0};
    mi_athlete.ffm[1] = (dnmx_feature_for_mixin){&run, mi_athlete.skills + 1, 0, 0};
    mi_athlete.info.init = init_athlete;
    mi_athlete.info.compare = dnmx_mixin_common_cmp_func;
    mi_athlete.info.features = mi_athlete.ffm;
    mi_athlete.info.num_features = 2;

    DNMX_COMMON_INIT_MI(mi_shooter.info, shooter);
    mi_shooter.skills[0] = 23;
    mi_shooter.ffm[0] = (dnmx_feature_for_mixin){&shoot, mi_shooter.skills, 0, 0};
    mi_shooter.info.features = mi_shooter.ffm;
    mi_shooter.info.num_features = 1;

    DNMX_COMMON_INIT_MI(mi_jumper.info, jumper);
    mi_jumper.skills[0] = -5;
    mi_jumper.ffm[0] = (dnmx_feature_for_mixin){&jump, mi_jumper.skills, 1, 0};
    mi_jumper.info.features = mi_jumper.ffm;
    mi_jumper.info.num_features = 1;

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings){0}, 0, NULL);
    dnmx_register_mixin(dom, &mi_warrior.info);
    dnmx_register_mixin(dom, &mi_athlete.info);
    dnmx_register_mixin(dom, &mi_shooter.info);
    dnmx_register_mixin(dom, &mi_jumper.info);

    const dnmx_mixin_info* ar_aw[] = {&mi_athlete.info, &mi_warrior.info};
    dnmx_type_handle taw = dnmx_get_type(dom, ar_aw, _countof(ar_aw));
    T_NOT_NULL(taw);

    const dnmx_mixin_info* ar_as[] = {&mi_athlete.info, &mi_shooter.info};
    dnmx_type_handle tas = dnmx_get_type(dom, ar_as, _countof(ar_as));
    T_NOT_NULL(tas);

    const dnmx_mixin_info* ar_s[] = {&mi_shooter.info};
    dnmx_type_handle ts = dnmx_get_type(dom, ar_s, _countof(ar_s));
    T_NOT_NULL(ts);

    const dnmx_mixin_info* ar_aj[] = {&mi_athlete.info, &mi_jumper.info};
    dnmx_type_handle taj = dnmx_get_type(dom, ar_aj, _countof(ar_aj));
    T_NOT_NULL(taj);

    dnmx_object_handle clone = NULL;
    {
        dnmx_object_handle obj = dnmx_create_object_default(taw);

        T_NOT_NULL(obj);
        CHECK(dnmx_object_get_domain(obj) == dom);
        CHECK(dnmx_object_get_type(obj) == taw);
        CHECK(dnmx_object_num_mixins(obj) == 2);
        CHECK_FALSE(dnmx_object_is_empty(obj));

        athlete* ath = dnmx_object_get_mut(obj, &mi_athlete.info);
        T_NOT_NULL(ath);
        CHECK(ath == dnmx_object_get_by_name(obj, dnmx_make_sv_lit("athlete")));
        CHECK(ath == dnmx_object_get_at(obj, 0));
        CHECK(ath == dnmx_object_get(obj, &mi_athlete.info));
        CHECK(ath == dnmx_object_get_mut_by_name(obj, dnmx_make_sv_lit("athlete")));
        CHECK(ath == dnmx_object_get_mut_at(obj, 0));

        CHECK(ath->speed == 10);
        T_SV_EXPECT("pernik", ath->target_name);

        const warrior* war = dnmx_object_get(obj, &mi_warrior.info);
        T_NOT_NULL(war);
        CHECK(war == dnmx_object_get_by_name(obj, dnmx_make_sv_lit("warrior")));
        CHECK(war == dnmx_object_get_at(obj, 1));
        CHECK(war == dnmx_object_get_mut(obj, &mi_warrior.info));
        CHECK(war == dnmx_object_get_mut_by_name(obj, dnmx_make_sv_lit("warrior")));
        CHECK(war == dnmx_object_get_mut_at(obj, 1));

        CHECK(war->speed == 0);
        CHECK(war->target_x == 0);
        CHECK(war->target_y == 0);

        ath->speed = 53;

        clone = dnmx_create_object_copy(obj);
        CHECK(dnmx_object_equals(clone, obj));

        dnmx_int_result cmp_res = dnmx_object_compare(clone, obj);
        CHECK(cmp_res.error == dnmx_result_success);
        CHECK(cmp_res.result == 0);

        dnmx_object_handle obj2 = dnmx_create_object_usurp(obj);
        CHECK(dnmx_object_is_empty(obj));
        dnmx_destroy_object(obj);
        obj = NULL;

        CHECK(ath == dnmx_object_get(obj2, &mi_athlete.info));
        ath->speed = 0;
        cmp_res = dnmx_object_compare(clone, obj2);
        CHECK(cmp_res.result > 0);

        dnmx_object_clear(obj2);
        CHECK(dnmx_object_is_empty(obj2));
        dnmx_destroy_object(obj2);
    }

    {
        dnmx_object_reset_type(clone, tas);
        CHECK(dnmx_object_get_type(clone) == tas);

        athlete* ath = dnmx_object_get_mut(clone, &mi_athlete.info);
        CHECK(ath->speed == 53);
        T_SV_EXPECT("pernik", ath->target_name);
    }

    {
        dnmx_object_handle obj = dnmx_create_object_default(taw);
        dnmx_object_seal(obj);

        CHECK_FALSE(dnmx_object_clear(obj));

        CHECK_FALSE(dnmx_object_is_empty(obj));
        CHECK(dnmx_object_get_type(obj) == taw);
        dnmx_destroy_object(obj);
    }

    dnmx_destroy_object(clone);
    dnmx_destroy_domain(dom);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(empty);
    RUN_TEST(simple);
    return UNITY_END();
}