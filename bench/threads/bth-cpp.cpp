// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bth-common.hpp"

virt_component::~virt_component() = default;
int virt_component::get_value(int) const {
    return 0;
}

namespace {
struct value_component final : public virt_component {
    int v;
    explicit value_component(int v) : v(v) {}
    int get_value(int arg) const override {
        return v + arg;
    }
};
struct filler_component final : public virt_component {
    int data = 0;
};
struct extra_component final : public virt_component {
    int data = 0;
};
}

void entity::add_extra() {
    components.push_back(std::make_unique<extra_component>());
}

void entity::remove_extra() {
    components.pop_back();
}

entity make_entity(int v) {
    entity ret;
    ret.components.reserve(4);
    ret.components.push_back(std::make_unique<value_component>(v));
    ret.components.push_back(std::make_unique<filler_component>());
    ret.components.push_back(std::make_unique<filler_component>());
    return ret;
}
