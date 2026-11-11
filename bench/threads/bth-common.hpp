// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/object.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <memory>
#include <vector>

// defined in separate compilation units from the benchmarks so that calls can't be inlined or devirtualized

// C++

struct virt_component {
    virtual ~virt_component();
    virtual int get_value(int arg) const;
};

// an entity with 3 polymorphic components, the first of which returns its value
struct entity {
    std::vector<std::unique_ptr<virt_component>> components;
    int get_value(int arg) const {
        return components.front()->get_value(arg);
    }
    void add_extra();
    void remove_extra();
};

entity make_entity(int v);

// DynaMix

// returns v + arg
DYNAMIX_DECLARE_MSG(get_value_msg, get_value, int, (const dynamix::object&, int));

// object with 3 mixins, the first of which implements get_value_msg
dynamix::object make_obj(int v);

// add and remove a 4th mixin
void add_extra(dynamix::object& obj);
void remove_extra(dynamix::object& obj);
