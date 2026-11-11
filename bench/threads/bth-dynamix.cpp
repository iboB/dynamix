// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bth-common.hpp"

#include <dynamix/declare_domain.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/mutate.hpp>

struct bthr;
DYNAMIX_DECLARE_DOMAIN(bthr);
DYNAMIX_DEFINE_DOMAIN(bthr);

DYNAMIX_MAKE_FUNC_TRAITS(get_value);
DYNAMIX_DEFINE_MSG(get_value_msg, unicast, get_value, int, (const dynamix::object&, int));

struct value_holder {
    int v = 0;
    int get_value(int arg) const { return v + arg; }
};
struct filler_a { int data = 0; };
struct filler_b { int data = 0; };
struct extra { int data = 0; };

DYNAMIX_DECLARE_MIXIN(value_holder);
DYNAMIX_DECLARE_MIXIN(filler_a);
DYNAMIX_DECLARE_MIXIN(filler_b);
DYNAMIX_DECLARE_MIXIN(extra);

DYNAMIX_DEFINE_MIXIN(bthr, value_holder).implements<get_value_msg>();
DYNAMIX_DEFINE_MIXIN(bthr, filler_a);
DYNAMIX_DEFINE_MIXIN(bthr, filler_b);
DYNAMIX_DEFINE_MIXIN(bthr, extra);

dynamix::object make_obj(int v) {
    using namespace dynamix;
    object ret(g::get_domain<bthr>());
    mutate(ret, add<value_holder>(value_holder{v}), add<filler_a>(), add<filler_b>());
    return ret;
}

void add_extra(dynamix::object& obj) {
    dynamix::mutate(obj, dynamix::add<extra>());
}

void remove_extra(dynamix::object& obj) {
    dynamix::mutate(obj, dynamix::remove<extra>());
}
