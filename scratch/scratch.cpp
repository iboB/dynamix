// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <iostream>
#include <dynamix/object.hpp>
#include <dynamix/dbg_dmp.hpp>
#include "../test/test_data.hpp"

int main() {
    dynamix::domain dom("scratch");
    test_data t;
    t.register_all_mixins(dom);
    t.create_types(dom);
    t.create_more_types(dom);
    dynamix::util::dbg_dmp(std::cout, dom);
}
