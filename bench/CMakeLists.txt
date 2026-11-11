# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.10.0)

# Instrument the measured parts of benchmarks for cachegrind and add cachegrind targets
# Linux only. Requires valgrind 3.22 or later. See cachegrind/README.md
option(DYNAMIX_BENCH_CACHEGRIND "DynaMix: instrument benchmarks for cachegrind and add cachegrind targets" OFF)

add_library(dynamix-bench-common INTERFACE)
target_include_directories(dynamix-bench-common INTERFACE common)
target_link_libraries(dynamix-bench-common INTERFACE picobench::picobench)

if(DYNAMIX_BENCH_CACHEGRIND)
    find_path(DYNAMIX_VALGRIND_INCLUDE_DIR valgrind/cachegrind.h)
    find_program(DYNAMIX_VALGRIND valgrind)
    find_program(DYNAMIX_RUBY ruby)
    if(NOT DYNAMIX_VALGRIND_INCLUDE_DIR OR NOT DYNAMIX_VALGRIND OR NOT DYNAMIX_RUBY)
        message(FATAL_ERROR "DYNAMIX_BENCH_CACHEGRIND requires valgrind (with headers) and ruby")
    endif()
    target_include_directories(dynamix-bench-common SYSTEM INTERFACE ${DYNAMIX_VALGRIND_INCLUDE_DIR})
    target_compile_definitions(dynamix-bench-common INTERFACE DYNAMIX_BENCH_CACHEGRIND=1)
endif()

# smoke test: a quick run with a small number of iterations and a single sample
# this checks that the benchmarks work and that they produce consistent results
set(DYNAMIX_BENCH_SMOKE_ARGS --iters=256 --samples=1)

macro(dynamix_benchmark name)
    set(tgt bench-dynamix-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_sources(${tgt} PRIVATE ${ARGN})
    target_link_libraries(${tgt} dynamix::dynamix dynamix-bench-common)
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
    add_custom_target(
        smoke-${tgt}
        COMMAND ${tgt} ${DYNAMIX_BENCH_SMOKE_ARGS}
    )
    set_target_properties(smoke-${tgt} PROPERTIES FOLDER bench)
endmacro()

add_subdirectory(msg-unicast)
add_subdirectory(msg-multicast)
add_subdirectory(type-creation)
add_subdirectory(msg-call)
add_subdirectory(object-lifetime)
add_subdirectory(memory)
add_subdirectory(threads)

# single-threaded benchmarks
# the multi-threaded ones are excluded from the smoke test and cachegrind
# as their results are only meaningful on a machine with enough cores and with real timing
set(DYNAMIX_ST_BENCHMARKS
    msg-unicast
    msg-multicast
    type-creation
    msg-call
    object-lifetime
)

add_custom_target(smoke-bench-dynamix)
set_target_properties(smoke-bench-dynamix PROPERTIES FOLDER bench)
foreach(b ${DYNAMIX_ST_BENCHMARKS} memory)
    add_dependencies(smoke-bench-dynamix smoke-bench-dynamix-${b})
endforeach()

if(DYNAMIX_BENCH_CACHEGRIND)
    set(cg_exes)
    foreach(b ${DYNAMIX_ST_BENCHMARKS})
        list(APPEND cg_exes $<TARGET_FILE:bench-dynamix-${b}>)
    endforeach()

    set(cg_command
        ${DYNAMIX_RUBY} ${CMAKE_CURRENT_SOURCE_DIR}/cachegrind/cachegrind.rb
        --valgrind ${DYNAMIX_VALGRIND}
        --baseline ${CMAKE_CURRENT_SOURCE_DIR}/cachegrind/baseline.json
        --output ${CMAKE_CURRENT_BINARY_DIR}/cachegrind-results.json
    )

    # run the benchmarks with cachegrind and compare the results with the baseline
    add_custom_target(
        cachegrind-bench-dynamix
        COMMAND ${cg_command} ${cg_exes}
        USES_TERMINAL
    )
    # run the benchmarks with cachegrind and overwrite the baseline
    add_custom_target(
        cachegrind-bench-dynamix-update-baseline
        COMMAND ${cg_command} --update ${cg_exes}
        USES_TERMINAL
    )
    foreach(b ${DYNAMIX_ST_BENCHMARKS})
        add_dependencies(cachegrind-bench-dynamix bench-dynamix-${b})
        add_dependencies(cachegrind-bench-dynamix-update-baseline bench-dynamix-${b})
    endforeach()
    set_target_properties(cachegrind-bench-dynamix cachegrind-bench-dynamix-update-baseline PROPERTIES FOLDER bench)
endif()
