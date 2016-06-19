# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE tests *.cpp)

foreach(source ${tests})
    get_filename_component(test ${source} NAME_WE)
    set(executable "test_${test}")

    add_executable(${executable} ${source})
    target_link_libraries(${executable} dynamix)
    set_target_properties(${executable} PROPERTIES FOLDER tests)
    add_test(NAME ${test} COMMAND $<TARGET_FILE:${executable}>)
endforeach()
