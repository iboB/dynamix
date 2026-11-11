// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/declare_type_class.hpp>
#include <dynamix/msg/declare_msg.hpp>
#include <dynamix/object.hpp>

#include <cstdint>
#include <memory>
#include <vector>

// the classes and functions here are defined in separate compilation units from the benchmarks
// so that the calls can't be inlined in the measured loops

// plain C++
// the C++ objects have the same amount of data as the DynaMix ones (10 ints)
// so that the memory footprint of the data is the same

struct plain_value {
    int v;
    int other[9];
    int get_value(int arg) const; // not virtual
};

struct virt_base {
    virtual ~virt_base();
    virtual int get_value(int arg) const = 0;
};

std::unique_ptr<plain_value> make_plain_value(int v);
std::unique_ptr<virt_base> make_virt_value(int v);

// dynamix

struct bcall;
DYNAMIX_DECLARE_DOMAIN(bcall);

// returns v + arg
DYNAMIX_DECLARE_MSG(get_value_msg, get_value, int, (const dynamix::object&, int));

// return arg
// ident_msg is implemented by a mixin, ident_default_msg only has a default implementation
DYNAMIX_DECLARE_MSG(ident_msg, ident, int, (const dynamix::object&, int));
DYNAMIX_DECLARE_MSG(ident_default_msg, ident_default, int, (const dynamix::object&, int));

// implemented by tagged
DYNAMIX_DECLARE_MSG(tag_msg, tag, int, (const dynamix::object&));

// the mixin which holds the value
struct value_holder {
    int v = 0;
    int get_value(int arg) const; // implements get_value_msg
    int ident(int arg) const; // implements ident_msg
};
DYNAMIX_DECLARE_MIXIN(value_holder);

// optional mixin which is used for queries
struct tagged {
    int tag() const;
};
DYNAMIX_DECLARE_MIXIN(tagged);

// type class: has tagged
DYNAMIX_DECLARE_TYPE_CLASS(is_tagged);

// objects with 10 mixins: 9 fillers, each implementing a filler message of its own, and value_holder in the middle
dynamix::object make_obj(int v);

// the same, but with a mixin with a higher bid at the back,
// which implements get_value_msg by calling the next implementation (that of value_holder)
dynamix::object make_obj_with_decorator(int v);

// the same as make_obj, but with tagged at the back of the type
dynamix::object make_tagged_obj(int v);
