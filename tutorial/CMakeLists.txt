# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 2.8.7)

file(GLOB tuts *.cpp)

foreach(source ${tuts})
    get_filename_component(tut ${source} NAME_WE)
    set(executable "tut_${tut}")

    add_executable(${executable} ${source})
    target_link_libraries(${executable} dynamix)
    set_target_properties(${executable} PROPERTIES FOLDER tutorials)
endforeach()
