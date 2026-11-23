# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.11.0)

add_library(dynamix-bench-common STATIC
    common/bench-main.cpp
)
target_link_libraries(dynamix-bench-common PUBLIC picobench::picobench)
target_include_directories(dynamix-bench-common INTERFACE common)

function(dynamix_benchmark)
    cmake_parse_arguments(ARG "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    set(tgt bench-dynamix-${ARG_NAME})

    # executable
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARG_SOURCES})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_link_libraries(${tgt} dynamix::dynamix dynamix-bench-common ${ARG_LIBRARIES})

    # run target
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
endfunction()

add_subdirectory(msg-unicast)
add_subdirectory(msg-multicast)
add_subdirectory(type-creation)
add_subdirectory(object-lifetime)
add_subdirectory(multi-threaded)
add_subdirectory(memory)
add_subdirectory(msg-call)
