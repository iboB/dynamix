// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-common.hpp"
#include <bench-rand.hpp>

std::vector<op_data> get_ops_combo(std::minstd_rand& rnd) {
    auto num_ops = uint32_t(rnd() % 5 + 2); // generate between 2 and 6 ops
    auto ops = dbench::pick_unique(num_ops, ops_count, rnd); // unique ops in random order

    std::vector<op_data> ret;
    ret.reserve(ops.size());
    for (auto op : ops) {
        auto a = uint32_t(rnd() % 10 + 1);
        auto b = uint32_t(rnd() % 10 + 1);
        ret.push_back({op_id(op), a, b});
    }
    return ret;
}
