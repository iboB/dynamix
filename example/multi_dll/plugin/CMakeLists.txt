# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.1)

add_library(plugin SHARED plugin.cpp)

target_link_libraries(plugin
    dynamix
    dynlib
    common
)

set_target_properties(plugin PROPERTIES FOLDER examples/multi_dll)
