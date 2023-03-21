# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.02)

add_subdirectory(picobench-main)

macro(dynamix_benchmark name)
    set(tgt bench-dynamix-${name})
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARGN})
    target_link_libraries(${tgt} dynamix::dynamix picobench-main)
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
endmacro()

add_subdirectory(msg-unicast)
