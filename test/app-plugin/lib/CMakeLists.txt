# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_shared_lib(dynamix-test-app-plugin-lib TAP
    tap-core-mixins.cpp
    tap-messages.cpp
    tap-domain.cpp
    tap-object.cpp
)
target_link_libraries(dynamix-test-app-plugin-lib PUBLIC dynamix::dynamix)
target_include_directories(dynamix-test-app-plugin-lib INTERFACE .)
