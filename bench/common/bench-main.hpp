// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <picobench/picobench.hpp>
#include <vector>

#if DYNAMIX_BENCH_CACHEGRIND
#include <valgrind/cachegrind.h>
#endif

namespace dbench {

// use this instead of picobench::scope for the measured part of benchmarks
// when built with DYNAMIX_BENCH_CACHEGRIND it also enables cachegrind instrumentation for the scope,
// so that when run with `valgrind --tool=cachegrind --instr-at-start=no` only the measured part is counted
// (see cachegrind/README.md)
class scope {
public:
    explicit scope(picobench::state& pb) : m_scope(pb) {
#if DYNAMIX_BENCH_CACHEGRIND
        CACHEGRIND_START_INSTRUMENTATION;
#endif
    }
    ~scope() {
#if DYNAMIX_BENCH_CACHEGRIND
        CACHEGRIND_STOP_INSTRUMENTATION;
#endif
    }
    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;
private:
    picobench::scope m_scope;
};

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
