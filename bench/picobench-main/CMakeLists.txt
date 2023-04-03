# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_library(picobench-main STATIC picobench-main.cpp)
set_target_properties(picobench-main PROPERTIES FOLDER bench)
target_link_libraries(picobench-main PUBLIC picobench::picobench)
