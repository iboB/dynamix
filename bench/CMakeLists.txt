# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.11.0)

# unstrument the measured parts of benchmarks for cachegrind and add cachegrind targets
# Linux only, valgrind 3.22 or later
option(DYNAMIX_BENCH_CACHEGRIND "DynaMix: instrument benchmarks for cachegrind and add cachegrind targets" OFF)

add_library(dynamix-bench-common STATIC
    common/bench-main.cpp
)
target_link_libraries(dynamix-bench-common PUBLIC picobench::picobench)
target_include_directories(dynamix-bench-common INTERFACE common)

if(DYNAMIX_BENCH_CACHEGRIND)
    find_path(DYNAMIX_VALGRIND_INCLUDE_DIR valgrind/cachegrind.h)
    find_program(DYNAMIX_VALGRIND valgrind)
    find_program(DYNAMIX_RUBY ruby)
    if(NOT DYNAMIX_VALGRIND_INCLUDE_DIR OR NOT DYNAMIX_VALGRIND OR NOT DYNAMIX_RUBY)
        message(FATAL_ERROR "DYNAMIX_BENCH_CACHEGRIND requires valgrind (with headers) and ruby")
    endif()
    target_include_directories(dynamix-bench-common SYSTEM PUBLIC ${DYNAMIX_VALGRIND_INCLUDE_DIR})
    target_compile_definitions(dynamix-bench-common PUBLIC BENCH_CACHEGRIND=1)
endif()

# smoke test: a quick run with a small number of iterations and a single sample
# this checks that the benchmarks work
set(DYNAMIX_BENCH_SMOKE_ARGS --iters=1024 --samples=1)

add_custom_target(smoke-bench-dynamix)
set_target_properties(smoke-bench-dynamix PROPERTIES FOLDER bench)

function(dynamix_benchmark)
    cmake_parse_arguments(ARG "SKIP_SMOKE_TEST" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    set(tgt bench-dynamix-${ARG_NAME})

    # executable
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARG_SOURCES})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_link_libraries(${tgt} dynamix::dynamix dynamix-bench-common ${ARG_LIBRARIES})

    if(NOT ARG_SKIP_SMOKE_TEST)
        # smoke target
        add_custom_target(
            smoke-${tgt}
            COMMAND ${tgt} ${DYNAMIX_BENCH_SMOKE_ARGS}
        )
        set_target_properties(smoke-${tgt} PROPERTIES FOLDER bench)

        add_dependencies(smoke-bench-dynamix smoke-${tgt})
    endif()
endfunction()

add_subdirectory(msg-unicast)
add_subdirectory(msg-multicast)
add_subdirectory(msg-call)
add_subdirectory(type-creation)
add_subdirectory(object-lifetime)

add_subdirectory(multi-threaded)
add_subdirectory(memory)

if(DYNAMIX_BENCH_CACHEGRIND)
    # only single-threaded perf benchmarks here
    # no multi-threaded as cachegrind is not reliable for multi-threaded programs
    # no point in adding report benchmarks as they don't measure perf
    set(benchmarks
        msg-unicast
        msg-multicast
        msg-call
        type-creation
        object-lifetime
    )

    set(exes)
    foreach(b ${benchmarks})
        list(APPEND exes $<TARGET_FILE:bench-dynamix-${b}>)
    endforeach()

    set(cg_cmd
        ${DYNAMIX_RUBY} ${CMAKE_CURRENT_SOURCE_DIR}/cachegrind/harness.rb
        --valgrind ${DYNAMIX_VALGRIND}
        --baseline ${CMAKE_CURRENT_SOURCE_DIR}/cachegrind/baseline.json
        --output ${CMAKE_CURRENT_BINARY_DIR}/cachegrind-results.json
    )

    add_custom_target(
        cachegrind-bench-dynamix
        COMMAND ${cg_cmd} ${exes}
        USES_TERMINAL
    )
    add_custom_target(
        cachegrind-bench-dynamix-update-baseline
        COMMAND ${cg_cmd} --update-baseline ${exes}
    )
    foreach(b ${benchmarks})
        add_dependencies(cachegrind-bench-dynamix bench-dynamix-${b})
        add_dependencies(cachegrind-bench-dynamix-update-baseline bench-dynamix-${b})
    endforeach()
    set_target_properties(
        cachegrind-bench-dynamix
        cachegrind-bench-dynamix-update-baseline
        PROPERTIES FOLDER bench
    )
endif()
