# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-scratch
    scratch.cpp
)
target_link_libraries(dynamix-scratch dynamix::dynamix doctest::util doctest::doctest)
set_target_properties(dynamix-scratch PROPERTIES FOLDER scratch)

add_executable(dynamix-c-scratch
    c-scratch.c
)
target_link_libraries(dynamix-c-scratch dynamix::dynamix)
set_target_properties(dynamix-c-scratch PROPERTIES FOLDER scratch)
