# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

add_executable(scratch scratch.cpp)
target_link_libraries(scratch dynamix)
set_target_properties(scratch PROPERTIES FOLDER scratch)
