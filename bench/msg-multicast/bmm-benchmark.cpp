// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-msg.hpp"
#include "bmm-std_func.hpp"
#include "bmm-virtual.hpp"
#include "bmm-msg.hpp"

#include <picobench/picobench.hpp>

constexpr uint32_t seed = 666;

template <typename Accumulator>
void benchmark(Accumulator (*factory)(std::minstd_rand&), picobench::state& pb) {
    std::minstd_rand rng(seed);

    std::vector<Accumulator> vec;
    vec.reserve(pb.iterations());
    for (int i = 0; i < pb.iterations(); ++i) {
        vec.push_back(factory(rng));
    }

    uint64_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto& a : vec) {
        sum += a.accumulate(rng() % 10);
    }
    pb.set_result(uintptr_t(sum));
}

void virt(picobench::state& pb) {
    benchmark<virt_accumulator>(make_virt_accumulator, pb);
}

PICOBENCH(virt).baseline();

void std_func(picobench::state& pb) {
    benchmark<func_accumulator>(make_func_accumulator, pb);
}

PICOBENCH(std_func);

void dynamix_msg(picobench::state& pb) {
    benchmark<dobject>(make_msg_accumulator, pb);
}

PICOBENCH(dynamix_msg);
