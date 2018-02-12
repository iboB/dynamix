# DynaMix
# Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
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
        if(NOT DYNAMIX_SHARED_LIB)
            # don't add plugins if DynaMix is a static library
            continue()
        endif()
        
        set(lib "test_${test}")

        add_library(${lib} SHARED ${source})
        target_link_libraries(${lib} dynamix)
        set_target_properties(${lib} PROPERTIES FOLDER tests)
    else()
        if(NOT DYNAMIX_SHARED_LIB AND test STREQUAL "loader")
            # loader test can only work when DynaMix is a shared library
            continue()
        endif()
        
        set(executable "test_${test}")

        add_executable(${executable} ${source})
        target_link_libraries(${executable} dynamix)
        set_target_properties(${executable} PROPERTIES FOLDER tests)
        add_test(NAME ${test} COMMAND $<TARGET_FILE:${executable}>)
    endif()
endforeach()

if(DYNAMIX_SHARED_LIB)
    # custom deps
    target_link_libraries(test_loader test_dynlib_A)
    target_link_libraries(test_plugin_A test_dynlib_A)
    target_link_libraries(test_plugin_Amod test_dynlib_A)
    target_link_libraries(test_plugin_B test_dynlib_A)

    if(NOT WIN32)
        target_link_libraries(test_loader dl)
    endif()
endif()