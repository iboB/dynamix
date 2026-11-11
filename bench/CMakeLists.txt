# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.10.0)

add_library(dynamix-bench-common INTERFACE)
target_include_directories(dynamix-bench-common INTERFACE common)
target_link_libraries(dynamix-bench-common INTERFACE picobench::picobench)

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
endmacro()

add_subdirectory(msg-unicast)
add_subdirectory(msg-multicast)
add_subdirectory(type-creation)
add_subdirectory(msg-call)
add_subdirectory(object-lifetime)
add_subdirectory(memory)
add_subdirectory(threads)
