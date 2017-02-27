# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 2.8.7)

set(sources)
src_group("src" sources
    config.hpp
    internal.hpp
    dynlib_pch.hpp
    dynlib_messages.hpp
    dynlib_messages.cpp
    dynlib_mixin.hpp
    dynlib_mixin.cpp
    dynlib_mixin_fwd.hpp
)

add_library(dynlib SHARED ${sources})

target_link_libraries(dynlib
    dynamix
)

set_target_properties(dynlib PROPERTIES FOLDER examples/multi_dll)
