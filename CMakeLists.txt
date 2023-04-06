# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(dynamix
    LANGUAGES C CXX
)

# cpm
include(./get_cpm.cmake)

#######################################
# cmake lib
CPMAddPackage(gh:iboB/icm@1.4.4)

list(APPEND CMAKE_MODULE_PATH
    "${icm_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

set(ICM_DEV_C_STANDARD 11)
set(ICM_DEV_CXX_STANDARD 17)

include(icm_dev_mode)

#######################################
# options

option(DYNAMIX_STATIC "DynaMix: build as a static library" OFF)
option(DYNAMIX_BUILD_V1COMPAT "DynaMix: build v1 compatibility lib" ${ICM_DEV_MODE})
option(DYNAMIX_BUILD_TESTS "DynaMix: enable testing and build tests" ${ICM_DEV_MODE})
option(DYNAMIX_BUILD_EXAMPLES "DynaMix: build examples" ${ICM_DEV_MODE})
option(DYNAMIX_BUILD_SCRATCH "DynaMix: build scratch project (dev experiments)" ${ICM_DEV_MODE})
option(DYNAMIX_BUILD_TUTORIALS "DynaMix: build tutorials" ${ICM_DEV_MODE})
option(DYNAMIX_BUILD_BENCH "DynaMix: build benchmarks" ${ICM_DEV_MODE})

# Off by default since it's slow
option(DYNAMIX_BUILD_COMPILER_BENCH "DynaMix: build compilation time benchmarks (may be really slow)" OFF)

#######################################
# cfg
if(ICM_STATIC_LIBS OR DYNAMIX_STATIC)
    set(DYNAMIX_SHARED FALSE)
else()
    set(DYNAMIX_SHARED TRUE)
endif()

#######################################
# code

add_subdirectory(code)

if(DYNAMIX_BUILD_V1COMPAT)
    add_subdirectory(v1compat)
endif()

if(DYNAMIX_BUILD_SCRATCH)
    add_subdirectory(scratch)
endif()

if(DYNAMIX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(DYNAMIX_BUILD_BENCH)
    add_subdirectory(bench)
endif()

if(DYNAMIX_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
