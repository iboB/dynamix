# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 2.8.7)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(dynlib)
add_subdirectory(plugin)
add_subdirectory(executable)