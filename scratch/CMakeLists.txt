# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-scratch
    scratch.cpp
)

target_link_libraries(dynamix-scratch dynamix::dynamix)
