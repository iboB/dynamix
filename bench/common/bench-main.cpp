// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bench-main.hpp"

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

namespace dbench {
int run_bench(int argc, char* argv[], std::vector<int> default_iterations) {
    picobench::runner r;
    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations(std::move(default_iterations));
    r.parse_cmd_line(argc, argv);
    return r.run();
}
} // namespace dbench
