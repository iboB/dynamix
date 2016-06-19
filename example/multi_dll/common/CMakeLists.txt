# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.1)

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
add_precompiled_header(common common_pch.hpp common_pch.cpp)

set_target_properties(common PROPERTIES FOLDER examples/multi_dll)
