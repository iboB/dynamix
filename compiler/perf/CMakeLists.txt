# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

include(${CMAKE_CURRENT_SOURCE_DIR}/precompiled_header.cmake)

add_executable(cperf_msg
    messages.hpp
    messages.cpp
    messages_call.cpp
    messages_main.cpp
    pch.hpp)

add_precompiled_header(cperf_msg pch.hpp pch.cpp)

target_link_libraries(cperf_msg dynamix)
