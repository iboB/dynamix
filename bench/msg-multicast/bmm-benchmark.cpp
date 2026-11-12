// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-msg.hpp"
#include "bmm-std_func.hpp"
#include "bmm-virtual.hpp"

#include <picobench/picobench.hpp>
#include <bench-main.hpp>
#include <bench-rand.hpp>

constexpr uint32_t seed = 666;

template <typename Accumulator>
void benchmark(Accumulator (*factory)(dbench::rng_t&), picobench::state& pb) {
    dbench::rng_t rng(seed);
    const auto n = size_t(pb.iterations());

    std::vector<Accumulator> vec;
    vec.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        vec.push_back(factory(rng));
    }

    // generate all data before the measured part
    // args[i] is the argument for the i-th call (not for the i-th object)
    std::vector<uint32_t> args(n);
    for (auto& a : args) {
        a = rng() % 10;
    }

    const auto access_type = dbench::access(pb.benchmark_user_data());
    const auto order = dbench::make_access_order(n, access_type, rng);

    uint64_t sum = 0;

    /////////////////////////////////
    // benchmarked code
    for (auto i : pb) {
        sum += vec[order[i]].accumulate(args[i]);
    }
    pb.set_result(uintptr_t(sum));
}

void virt(picobench::state& pb) {
    benchmark<virt_accumulator>(make_virt_accumulator, pb);
}

void std_func(picobench::state& pb) {
    benchmark<func_accumulator>(make_func_accumulator, pb);
}

void dynamix_msg(picobench::state& pb) {
    benchmark<dobject>(make_msg_accumulator, pb);
}
// note that the memory layouts differ slightly:
// * virt: each accumulator has a vector of individually allocated ops
// * std_func: each accumulator has a vector of std::function-s (typically using the small buffer)
// * dynamix_msg: each object has its mixins in a single buffer

// objects are accessed in the order in which they were created
PICOBENCH_SUITE("multicast: sequential access");
PICOBENCH(virt).user_data(dbench::access_sequential).baseline();
PICOBENCH(std_func).user_data(dbench::access_sequential);
PICOBENCH(dynamix_msg).user_data(dbench::access_sequential);

// objects are accessed in random order
PICOBENCH_SUITE("multicast: shuffled access");
PICOBENCH(virt).user_data(dbench::access_shuffled).baseline();
PICOBENCH(std_func).user_data(dbench::access_shuffled);
PICOBENCH(dynamix_msg).user_data(dbench::access_shuffled);

int main(int argc, char* argv[]) {
    return dbench::run_bench(argc, argv, {1024, 16384, 65536});
}
