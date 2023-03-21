# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_library(picobench-main STATIC picobench-main.cpp)
target_link_libraries(picobench-main PUBLIC picobench::picobench)
