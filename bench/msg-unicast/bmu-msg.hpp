// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/common_mixin_init.hpp>
#include <dynamix/msg/declare_msg.hpp>

struct bench;
DYNAMIX_DECLARE_DOMAIN(bench);

DYNAMIX_DECLARE_MIXIN(struct square_mixin);
DYNAMIX_DECLARE_MIXIN(struct rect_mixin);
DYNAMIX_DECLARE_MIXIN(struct circle_mixin);
DYNAMIX_DECLARE_MIXIN(struct triangle_mixin);

struct mixin_square : public dynamix::common_mixin_init<square_mixin> {
    int side;
    mixin_square(int side) : side(side) {}
    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
};

struct mixin_rect : public dynamix::common_mixin_init<rect_mixin> {
    int a, b;
    mixin_rect(int a, int b) : a(a), b(b) {}
    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
};

struct mixin_circle : public dynamix::common_mixin_init<circle_mixin> {
    int radius;
    mixin_circle(int radius) : radius(radius) {}
    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
};

struct mixin_triangle : public dynamix::common_mixin_init<triangle_mixin> {
    int a, ha;
    mixin_triangle(int a, int ha) : a(a), ha(ha) {}
    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
};

DYNAMIX_DECLARE_MSG(area_msg, area, int, (const dynamix::object&, int));
