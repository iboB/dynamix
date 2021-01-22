# DynaMix
# Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

# demo configuration
# sets some configurations and enable some settings which don't make sense unless you're working
# on DynaMix itself
# (could also be used as a toolchain)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(DYNAMIX_TSAN "DynaMix: build with thread sanitizer on" OFF)
option(DYNAMIX_ASAN "DynaMix: build with address sanitizer on" OFF)
option(DYNAMIX_CLANG_TIDY "Dynamix: use clang tidy" OFF)

set(DEMO_SAN_FLAGS "")
if(MSVC)
    set(DEMO_WARNING_FLAGS "/w34100")
else()
    set(DEMO_WARNING_FLAGS "-Wall -Wextra -Wno-unused-variable")
    if(DYNAMIX_TSAN)
        set(DEMO_SAN_FLAGS "-fsanitize=thread -g")
    elseif(DYNAMIX_ASAN)
        set(DEMO_SAN_FLAGS "-fsanitize=address,undefined,leak -pthread -g")
    endif()

    if(DYNAMIX_CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY clang-tidy)
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEMO_WARNING_FLAGS} ${DEMO_SAN_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEMO_WARNING_FLAGS} ${DEMO_SAN_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DEMO_SAN_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${DEMO_SAN_FLAGS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED) # for thread test
