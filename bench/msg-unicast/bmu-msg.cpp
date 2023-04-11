// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmu-msg.hpp"
#include "pi.hpp"

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/object.hpp>

struct square_mixin {
    square_mixin(int side) : m_side(side) {}
    int area(int mod) const;
    int m_side;
};

struct rect_mixin {
    rect_mixin(int a, int b) : m_a(a), m_b(b) {}
    int area(int mod) const;
    int m_a, m_b;
};

struct circle_mixin {
    circle_mixin(int radius) : m_radius(radius) {}
    int area(int mod) const;
    int m_radius;
};

struct triangle_mixin {
    triangle_mixin(int a, int ha) : m_a(a), m_ha(ha) {}
    int area(int mod) const;
    int m_a, m_ha;
};

inline int square_mixin::area(int mod) const {
    return m_side * m_side * mod;
}

inline int rect_mixin::area(int mod) const {
    return m_a * m_b * mod;
}

inline int circle_mixin::area(int mod) const {
    double r3 = m_radius * m_radius * pi;
    return int(r3) * mod;
}

inline int triangle_mixin::area(int mod) const {
    return mod * m_a * m_ha / 2;
}

void mixin_square::do_init(dynamix::init_new_args args) {
    new (args.mixin_buf) square_mixin(side);
}

void mixin_rect::do_init(dynamix::init_new_args args) {
    new (args.mixin_buf) rect_mixin(a, b);
}

void mixin_circle::do_init(dynamix::init_new_args args) {
    new (args.mixin_buf) circle_mixin(radius);
}

void mixin_triangle::do_init(dynamix::init_new_args args) {
    new (args.mixin_buf) triangle_mixin(a, ha);
}

DYNAMIX_DEFINE_DOMAIN(bench);

DYNAMIX_MAKE_FUNC_TRAITS(area);

DYNAMIX_DEFINE_MIXIN(bench, square_mixin).implements<area_msg>();
DYNAMIX_DEFINE_MIXIN(bench, rect_mixin).implements<area_msg>();
DYNAMIX_DEFINE_MIXIN(bench, triangle_mixin).implements<area_msg>();
DYNAMIX_DEFINE_MIXIN(bench, circle_mixin).implements<area_msg>();

DYNAMIX_DEFINE_MSG(area_msg, unicast, area, int, (const dynamix::object&, int));

