# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

include_directories(common picobench)

set(common_sources)
src_group(common common_sources
    common/timer.cpp
    common/timer.hpp
)

set(message_perf_sources)
src_group(perf message_perf_sources
    message_perf/main.cpp
    message_perf/perf.cpp
    message_perf/perf.hpp
)

add_executable(message_perf
    ${message_perf_sources}
)

target_link_libraries(message_perf dynamix)
set_target_properties(message_perf PROPERTIES FOLDER performance)

set(mutation_perf_sources)
src_group(perf mutation_perf_sources
    mutation_perf/common.hpp
    mutation_perf/main.cpp
    mutation_perf/custom_alloc_mixin.cpp
    mutation_perf/custom_alloc_mixin.hpp
    mutation_perf/generated.cpp
    mutation_perf/generated.hpp
)

add_executable(mutation_perf
    ${common_sources}
    ${mutation_perf_sources}
)

target_link_libraries(mutation_perf dynamix)
set_target_properties(mutation_perf PROPERTIES FOLDER performance)
