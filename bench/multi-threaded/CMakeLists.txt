# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
set(CMAKE_CXX_STANDARD 20) # par is C++20 only

CPMAddPackage(gh:iboB/par@0.2.3)

dynamix_benchmark(
    NAME multi-threaded
    SOURCES
        bmt-benchmark.cpp
        bmt-cpp.cpp
        bmt-dynamix.cpp
    LIBRARIES
        par::par

    # for some reason this test hangs on the windows-latest github action runner,
    # so skip it for now
    SKIP_SMOKE_TEST
)
