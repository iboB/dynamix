# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
set(testPluginDir ${CMAKE_BINARY_DIR}/bin/test-plugins)

function(add_test_plugin name)
    set(tgt dynamix-test-plugin-${name})
    add_library(${tgt} MODULE ${ARGN})
    target_link_libraries(${tgt} PRIVATE dynamix::dynamix dynamix-test-app-plugin-lib)
    if(NOT WIN32)
        target_compile_options(${tgt} PRIVATE -fvisibility=hidden)
    endif()
    set_target_properties(${tgt} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${testPluginDir}
        FOLDER test
    )
endfunction()

add_subdirectory(lib)
add_subdirectory(exe)

add_subdirectory(plugin-a)
add_subdirectory(plugin-b)
add_subdirectory(plugin-a-mod)

