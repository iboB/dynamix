# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
find_package(Threads REQUIRED)

dynamix_benchmark(threads
    bth-benchmark.cpp
    bth-cpp.cpp
    bth-dynamix.cpp
)
target_link_libraries(bench-dynamix-threads Threads::Threads)
