# DynaMix
# Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE tests *.cpp)

foreach(source ${tests})
    get_filename_component(test ${source} NAME_WE)

    if(test MATCHES "^plugin_" OR test MATCHES "^dynlib_")
        set(lib "test_${test}")

        add_library(${lib} SHARED ${source})
        target_link_libraries(${lib} dynamix)
        set_target_properties(${lib} PROPERTIES FOLDER tests)
    else()
        set(executable "test_${test}")

        add_executable(${executable} ${source})
        target_link_libraries(${executable} dynamix)
        set_target_properties(${executable} PROPERTIES FOLDER tests)
        add_test(NAME ${test} COMMAND $<TARGET_FILE:${executable}>)
    endif()
endforeach()

# custom deps
target_link_libraries(test_loader test_dynlib_A)
target_link_libraries(test_plugin_A test_dynlib_A)
target_link_libraries(test_plugin_Amod test_dynlib_A)
target_link_libraries(test_plugin_B test_dynlib_A)

if(NOT WIN32)
    target_link_libraries(test_loader dl)
endif()
