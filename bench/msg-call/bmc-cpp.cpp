// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmc-common.hpp"

int plain_value::get_value(int arg) const {
    return v + arg;
}

virt_base::~virt_base() = default;

namespace {
struct virt_value final : public virt_base {
    int v;
    int other[9] = {};
    explicit virt_value(int v) : v(v) {}
    int get_value(int arg) const override {
        return v + arg;
    }
};
}

std::unique_ptr<plain_value> make_plain_value(int v) {
    return std::make_unique<plain_value>(plain_value{v, {}});
}

std::unique_ptr<virt_base> make_virt_value(int v) {
    return std::make_unique<virt_value>(v);
}
