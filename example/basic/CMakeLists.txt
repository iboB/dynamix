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
set_target_properties(basic_example PROPERTIES FOLDER examples)
