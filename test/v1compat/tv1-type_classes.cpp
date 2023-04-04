// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/v1compat/type_class.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("type classes");

DYNAMIX_V1_MESSAGE_0(void, move);
DYNAMIX_V1_MESSAGE_0(void, draw);
DYNAMIX_V1_MESSAGE_0(void, shoot);

DYNAMIX_V1_DECLARE_MIXIN(ghost);
DYNAMIX_V1_DECLARE_MIXIN(visible);

DYNAMIX_V1_DECLARE_MIXIN(soldier);
DYNAMIX_V1_DECLARE_MIXIN(tank);
DYNAMIX_V1_DECLARE_MIXIN(pacifist);
DYNAMIX_V1_DECLARE_MIXIN(cannon);

TEST_CASE("local") {
    using namespace dynamix::v1compat;

    object gc; mutate(gc).add<ghost>().add<cannon>();
    object vc; mutate(vc).add<visible>().add<cannon>();
    object gs; mutate(gs).add<ghost>().add<soldier>();
    object vt; mutate(vt).add<visible>().add<tank>();

    // v2!: type class constructor receives dnmx_type_handle
    // v2!: match func is noexcept
    type_class has_ghost([](dnmx_type_handle ht) noexcept {
        auto& ti = *dynamix::type::from_c_handle(ht);
        return ti.has<ghost>();
    });

    type_class move_and_shoot([](dnmx_type_handle ht) noexcept {
        auto& ti = *dynamix::type::from_c_handle(ht);
        // v2!: implements is always templated
        return ti.implements<move_msg_t>() && ti.implements<shoot_msg_t>();
    });

    type_class shoot_and_visible_and_tank([](dnmx_type_handle ht) noexcept {
        auto& ti = *dynamix::type::from_c_handle(ht);
        return ti.implements<shoot_msg_t>() && ti.implements<draw_msg_t>() && ti.has<tank>();
    });

    CHECK(gc.is_a(has_ghost));

    CHECK_FALSE(vc.is_a(has_ghost));
    CHECK_FALSE(vc.is_a(move_and_shoot));
    CHECK_FALSE(vc.is_a(shoot_and_visible_and_tank));

    CHECK(gs.is_a(has_ghost));
    CHECK(gs.is_a(move_and_shoot));

    CHECK_FALSE(vt.is_a(has_ghost));
    CHECK(vt.is_a(move_and_shoot));
    CHECK(vt.is_a(shoot_and_visible_and_tank));
}

const dynamix::v1compat::type_class move_and_shoot_and_ghost([](dnmx_type_handle ht) noexcept {
    auto& ti = *dynamix::type::from_c_handle(ht);
    return ti.implements<move_msg_t>()
        && ti.implements<shoot_msg_t>()
        && ti.has<ghost>();
}, true);

DYNAMIX_V1_TYPE_CLASS(move_and_ghost_and_tank);
DYNAMIX_V1_DEFINE_TYPE_CLASS(move_and_ghost_and_tank) {
    return type.implements<move_msg_t>()
        && type.has<ghost>()
        && type.has<tank>();
}

TEST_CASE("global") {
    using namespace dynamix::v1compat;
    object gt; mutate(gt).add<ghost>().add<tank>();
    // v2!: no matching_type_classes in type
    // CHECK(gt._type_info->_matching_type_classes.size() == 2);
    CHECK(gt.is_a(move_and_shoot_and_ghost));
    CHECK(gt.is_a<move_and_ghost_and_tank>());

    object gs; mutate(gs).add<ghost>().add<soldier>();
    // CHECK(gs._type_info->_matching_type_classes.size() == 1);
    CHECK(gs.is_a(move_and_shoot_and_ghost));
    CHECK_FALSE(gs.is_a<move_and_ghost_and_tank>());
}

DYNAMIX_V1_DEFINE_MESSAGE(move);
DYNAMIX_V1_DEFINE_MESSAGE(draw);
DYNAMIX_V1_DEFINE_MESSAGE(shoot);

class ghost {};
DYNAMIX_V1_DEFINE_MIXIN(ghost, dynamix::v1compat::none);

class visible {
public:
    void draw() {}
};
DYNAMIX_V1_DEFINE_MIXIN(visible, draw_msg);

class soldier {
public:
    void move() {}
    void shoot() {}
};
DYNAMIX_V1_DEFINE_MIXIN(soldier, move_msg & shoot_msg);

class tank {
public:
    void move() {}
    void shoot() {}
};
DYNAMIX_V1_DEFINE_MIXIN(tank, move_msg & shoot_msg);

class pacifist {
public:
    void move() {}
};
DYNAMIX_V1_DEFINE_MIXIN(pacifist, move_msg);

class cannon {
public:
    void shoot() {}
};
DYNAMIX_V1_DEFINE_MIXIN(cannon, shoot_msg);
