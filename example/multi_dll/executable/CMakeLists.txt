# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

set(sources)
src_group("src" sources
    main.cpp
    executable_pch.hpp
    object_factory.cpp
    object_factory.hpp
    exe_mixin.hpp
    exe_mixin.cpp
    exe_mixin_fwd.hpp
)

add_executable(executable ${sources})

target_link_libraries(executable
    dynamix
    dynlib
)

if(NOT MSVC)
    target_link_libraries(executable dl)
endif()

set_target_properties(executable PROPERTIES FOLDER examples/multi_dll)
