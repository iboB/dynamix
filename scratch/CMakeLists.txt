# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-scratch
    scratch.cpp
)
target_link_libraries(dynamix-scratch dynamix::dynamix)

add_executable(dynamix-c-scratch
    c-scratch.c
)
target_link_libraries(dynamix-c-scratch dynamix::dynamix)
