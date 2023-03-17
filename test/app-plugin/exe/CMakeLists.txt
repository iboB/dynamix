# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
dynamix_test(plugin
    SOURCES tap-exe-main.cpp
    LIBRARIES dynamix-test-app-plugin-lib ${CMAKE_DL_LIBS}
)
target_compile_definitions(test-dynamix-plugin PRIVATE -DPLUGIN_DIR="${testPluginDir}")
