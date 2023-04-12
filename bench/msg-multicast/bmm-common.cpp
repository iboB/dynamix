// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-common.hpp"
#include <itlib/flat_set.hpp>

void shuffle(std::vector<op_id>& vec, std::minstd_rand& rnd) {
    // don't use std::shuffle as it has different implementations in
    // different standard libraries and breaks the fuzz test determinism with a given seed
    for (int i = 0; i < 2; ++i) {
        for (auto& elem : vec) {
            std::swap(elem, vec[rnd() % vec.size()]);
        }
    }
}

std::vector<op_data> get_ops_combo(std::minstd_rand& rnd) {
    auto num_ops = rnd() % 5 + 2; // generate between 2 and 6 ops
    itlib::flat_set<op_id> set;
    while (set.size() != num_ops) set.insert(op_id(rnd() % ops_count)); // generate unique ops
    auto ops_shuf = std::move(set.modify_container());
    shuffle(ops_shuf, rnd); // but not sorted

    std::vector<op_data> ret;
    ret.reserve(ops_shuf.size());
    for (auto op : ops_shuf) {
        auto a = rnd() % 10 + 1;
        auto b = rnd() % 10 + 1;
        ret.push_back({op, a, b});
    }
    return ret;
}
