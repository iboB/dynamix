// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <picobench/picobench.hpp>
#include <vector>

namespace dbench {

// shared main for all benchmarks
// note that picobench uses a double dash for command line options: --iters=..., --samples=...
inline int run(int argc, char* argv[], std::vector<int> default_iterations) {
    picobench::runner r;
    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true); // compares benchmarks within a suite
    r.set_default_state_iterations(std::move(default_iterations));
    // picobench reports the fastest sample
    // more samples make the result less susceptible to noise
    r.set_default_samples(5);
    r.parse_cmd_line(argc, argv);
    return r.run();
}

} // namespace dbench
