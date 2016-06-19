# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 2.8.7)

set(sources)
src_group("precompiled" sources
    common_pch.cpp
    common_pch.hpp
)

src_group("src" sources
    messages.hpp
    messages.cpp
    mixin.hpp
    mixin.cpp
    mixin_fwd.hpp
)

add_library(common STATIC ${sources})
add_precompiled_header(common ${CMAKE_CURRENT_SOURCE_DIR}/common_pch.hpp ${CMAKE_CURRENT_SOURCE_DIR}/common_pch.cpp)

set_target_properties(common PROPERTIES FOLDER examples/multi_dll)

# To fix: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set_target_properties(common PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
