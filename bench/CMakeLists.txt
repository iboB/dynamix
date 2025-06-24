# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.08)

add_subdirectory(picobench-main)

macro(dynamix_benchmark name)
    set(tgt bench-dynamix-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_sources(${tgt} PRIVATE ${ARGN})
    target_link_libraries(${tgt} dynamix::dynamix picobench-main)
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
endmacro()

add_subdirectory(msg-unicast)
add_subdirectory(msg-multicast)
add_subdirectory(type-creation)
