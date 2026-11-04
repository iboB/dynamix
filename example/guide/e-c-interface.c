// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/c/c-interface.md
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>
#include <dnmx/mixin_info_util.h>
#include <dnmx/object.h>
#include <dnmx/type.h>
#include <dnmx/mutate.h>

#include <stdio.h>

// [features]
// features are described by feature infos
// here we use their payloads as functions, like messages in C++
dnmx_feature_info describe_feature; // multicast: void(const void* mixin)
dnmx_feature_info get_hp_feature; // unicast: int(const void* mixin)

typedef void (*describe_func)(const void* mixin);
typedef int (*get_hp_func)(const void* mixin);

// call a unicast: the top implementation
int get_hp(dnmx_object_handle obj) {
    dnmx_ftable_entry e = dnmx_ftable_at(obj->m_type, get_hp_feature.id);
    if (!e.begin) {
        // no implementation: use the default payload (which has a different signature here)
        return ((int (*)(dnmx_object_handle))get_hp_feature.default_payload)(obj);
    }
    get_hp_func f = (get_hp_func)e.begin->payload;
    return f(dnmx_object_unchecked_get_at(obj, e.begin->mixin_index));
}

// call a multicast: all implementations with the top bid, from top_bid_back to begin
void describe(dnmx_object_handle obj) {
    dnmx_ftable_entry e = dnmx_ftable_at(obj->m_type, describe_feature.id);
    if (!e.begin) return;
    for (const dnmx_ftable_payload* p = e.top_bid_back + 1; p != e.begin; ) {
        --p;
        describe_func f = (describe_func)p->payload;
        f(dnmx_object_unchecked_get_at(obj, p->mixin_index));
    }
}
// [features]

// [mixins]
// mixins are plain structs
typedef struct health {
    int hp;
} health;

typedef struct armor {
    int bonus;
} armor;

void health_describe(const void* m) { printf("health: %d\n", ((const health*)m)->hp); }
int health_get_hp(const void* m) { return ((const health*)m)->hp; }

void armor_describe(const void* m) { printf("armor: +%d\n", ((const armor*)m)->bonus); }
int armor_get_hp(const void* m) { return 100 + ((const armor*)m)->bonus; } // fake: armor adds to hp

int default_get_hp(dnmx_object_handle obj) { (void)obj; return 0; }

dnmx_feature_for_mixin health_features[2];
dnmx_feature_for_mixin armor_features[2];

dnmx_mixin_info health_info;
dnmx_mixin_info armor_info;
// [mixins]

// [mutate-funcs]
// a custom init function for a mixin in a mutation
dnmx_error_return_t init_health(dnmx_mutate_func_args args) {
    health* h = (health*)args.mixin_buf;
    h->hp = (int)args.user_data;
    return dnmx_result_success;
}
// [mutate-funcs]

int main(void) {
    // [domain]
    dnmx_domain_settings settings = { 0 };
    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("game"), settings, 0, NULL);
    // [domain]

    // [register]
    describe_feature = dnmx_make_feature_info();
    describe_feature.name = dnmx_make_sv_lit("describe");
    describe_feature.allow_clashes = true;

    get_hp_feature = dnmx_make_feature_info();
    get_hp_feature.name = dnmx_make_sv_lit("get_hp");
    get_hp_feature.allow_clashes = true;
    get_hp_feature.default_payload = (void*)default_get_hp;

    // DNMX_COMMON_INIT_MI sets the name, the size, and functions which treat the mixin as plain data
    DNMX_COMMON_INIT_MI(health_info, health);
    health_features[0] = (dnmx_feature_for_mixin){ .info = &describe_feature, .payload = (void*)health_describe };
    health_features[1] = (dnmx_feature_for_mixin){ .info = &get_hp_feature, .payload = (void*)health_get_hp };
    health_info.features = health_features;
    health_info.num_features = 2;
    dnmx_register_mixin(dom, &health_info);

    DNMX_COMMON_INIT_MI(armor_info, armor);
    armor_features[0] = (dnmx_feature_for_mixin){ .info = &describe_feature, .payload = (void*)armor_describe };
    armor_features[1] = (dnmx_feature_for_mixin){ .info = &get_hp_feature, .payload = (void*)armor_get_hp, .bid = 1 };
    armor_info.features = armor_features;
    armor_info.num_features = 2;
    dnmx_register_mixin(dom, &armor_info);
    // [register]

    // [use]
    dnmx_object_handle hero = dnmx_create_object_empty(dom);
    printf("hp: %d\n", get_hp(hero)); // default payload

    {
        dnmx_mutate_op ops[] = {
            {.op_type = dnmx_mutate_op_add, .mixin = &health_info, .init_override = init_health, .user_data = 80},
        };
        if (dnmx_mutate(hero, ops, 1) != dnmx_result_success) {
            printf("mutation failed\n");
            return 1;
        }
    }
    printf("hp: %d\n", get_hp(hero));

    {
        dnmx_mutate_op ops[] = {
            {.op_type = dnmx_mutate_op_add, .mixin_name = dnmx_make_sv_lit("armor")},
        };
        dnmx_mutate(hero, ops, 1);
    }
    ((armor*)dnmx_object_get_mut(hero, &armor_info))->bonus = 5;
    printf("hp: %d\n", get_hp(hero)); // armor has a higher bid
    describe(hero);
    // [use]

    // [cleanup]
    dnmx_destroy_object(hero);
    dnmx_unregister_mixin(dom, &armor_info);
    dnmx_unregister_mixin(dom, &health_info);
    dnmx_destroy_domain(dom);
    // [cleanup]
    return 0;
}
