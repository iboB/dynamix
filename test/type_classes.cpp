// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/core.hpp>
#include <dynamix/define_type_class.hpp>

#include "doctest/doctest.h"

TEST_SUITE_BEGIN("type classes");

DYNAMIX_MESSAGE_0(void, move);
DYNAMIX_MESSAGE_0(void, draw);
DYNAMIX_MESSAGE_0(void, shoot);

DYNAMIX_DECLARE_MIXIN(ghost);
DYNAMIX_DECLARE_MIXIN(visible);

DYNAMIX_DECLARE_MIXIN(soldier);
DYNAMIX_DECLARE_MIXIN(tank);
DYNAMIX_DECLARE_MIXIN(pacifist);
DYNAMIX_DECLARE_MIXIN(cannon);

TEST_CASE("local")
{
    using namespace dynamix;

    object gc; mutate(gc).add<ghost>().add<cannon>();
    object vc; mutate(vc).add<visible>().add<cannon>();
    object gs; mutate(gs).add<ghost>().add<soldier>();
    object vt; mutate(vt).add<visible>().add<tank>();

    type_class has_ghost = define_type_class([](const dynamix::object_type_info& ti) {
        return ti.has<ghost>();
    });

    //type_class move_and_shoot = define_type_class()
    //    .implements(move_msg)
    //    .implements(shoot_msg);

    //type_class shoot_and_visible_and_tank = define_type_class()
    //    .implements(move_msg)
    //    .has<tank>();

    CHECK(gc.is_a(has_ghost));

    //CHECK_FALSE(vc.is_a(has_ghost));
    //CHECK_FALSE(vc.is_a(move_and_shoot));
    //CHECK_FALSE(vc.is_a(shoot_and_visible_and_tank));

    //CHECK(gs.is_a(has_ghost));
    //CHECK(gs.is_a(move_and_shoot));

    //CHECK_FALSE(vt.is_a(has_ghost));
    //CHECK(vt.is_a(move_and_shoot));
    //CHECK(vt.is_a(shoot_and_visible_and_tank));
}

//const dynamix::type_class move_and_shoot_and_ghost = dynamix::register_type_class()
//    .implements(move_msg)
//    .implements(shoot_msg)
//    .has<ghost>();
//
//const dynamix::type_class move_and_ghost_and_tank = dynamix::register_type_class()
//    .implements(move_msg)
//    .has<ghost>()
//    .has<tank>();

//TEST_CASE("global")
//{
//    using namespace dynamix;
//    object gt; mutate(gt).add<ghost>().add<tank>();
//
//    CHECK(gt._type_info->_matching_type_classes.size() == 2);
//}

DYNAMIX_DEFINE_MESSAGE(move);
DYNAMIX_DEFINE_MESSAGE(draw);
DYNAMIX_DEFINE_MESSAGE(shoot);

class ghost {};
DYNAMIX_DEFINE_MIXIN(ghost, dynamix::none);

class visible
{
public:
    void draw() {}
};
DYNAMIX_DEFINE_MIXIN(visible, draw_msg);

class soldier
{
public:
    void move() {}
    void shoot() {}
};
DYNAMIX_DEFINE_MIXIN(soldier, move_msg & shoot_msg);

class tank
{
public:
    void move() {}
    void shoot() {}
};
DYNAMIX_DEFINE_MIXIN(tank, move_msg& shoot_msg);

class pacifist
{
public:
    void move() {}
};
DYNAMIX_DEFINE_MIXIN(pacifist, move_msg);

class cannon
{
public:
    void shoot() {}
};
DYNAMIX_DEFINE_MIXIN(cannon, shoot_msg);
