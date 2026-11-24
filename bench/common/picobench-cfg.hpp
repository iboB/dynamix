// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#if BENCH_CACHEGRIND
#   include <valgrind/cachegrind.h>
#   define PICOBENCH_START_INSTRUMENTATION CACHEGRIND_START_INSTRUMENTATION
#   define PICOBENCH_STOP_INSTRUMENTATION CACHEGRIND_STOP_INSTRUMENTATION
#endif

#include <picobench/picobench.hpp>

