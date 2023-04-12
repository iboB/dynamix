// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-msg.hpp"
#include "bmm-std_func.hpp"
#include "bmm-virtual.hpp"

#include <picobench/picobench.hpp>

constexpr uint32_t seed = 666;

void virt(picobench::state& pb) {
    std::minstd_rand rng(seed);

    std::vector<virt_accumulator> vec;
    vec.reserve(pb.iterations());
    for (int i = 0; i < pb.iterations(); ++i) {
        vec.push_back(make_virt_accumulator(rng));
    }

    uint64_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto& a : vec) {
        sum += a.accumulate(rng() % 10);
    }
    pb.set_result(sum);
}
