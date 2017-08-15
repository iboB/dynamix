# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

file(GLOB sources *.cpp *.hpp)

add_executable(basic_example ${sources})

target_link_libraries(basic_example dynamix)
add_precompiled_header(basic_example ${CMAKE_CURRENT_SOURCE_DIR}/basic.hpp ${CMAKE_CURRENT_SOURCE_DIR}/basic.cpp)
set_target_properties(basic_example PROPERTIES FOLDER examples)
