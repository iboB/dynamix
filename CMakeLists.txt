# DynaMix
# Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#

cmake_minimum_required(VERSION 3.2.2)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # if DynaMix is built as root, activate folders
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # is_demo is used below to enable the build of various subprojects
    # which typically wouldn't be built if DynaMix is a subdirectory of a project
    set(is_demo ON)
else()
    set(is_demo OFF)
endif()

project(dynamix)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

include(src_group)

option(DYNAMIX_SHARED_LIB "DynaMix: build as a shared library" ON)

option(DYNAMIX_BUILD_UNIT_TESTS "DynaMix: enable testing and build tests" ${is_demo})

option(DYNAMIX_BUILD_EXAMPLES "DynaMix: build examples" ${is_demo})
# Individual example settings. Only relevant if DYNAMIX_BUILD_EXAMPLES is on
option(DYNAMIX_EXAMPLE_HELLO_WORLD "DynaMix: build hello world example" ${is_demo})
option(DYNAMIX_EXAMPLE_BASIC "DynaMix: build basic example" ${is_demo})
option(DYNAMIX_EXAMPLE_MULTI_DLL "DynaMix: build multi_dll example" ${is_demo})

option(DYNAMIX_BUILD_SCRATCH "DynaMix: build scratch project" ${is_demo})

option(DYNAMIX_BUILD_TUTORIALS "DynaMix: build tutorials" ${is_demo})

# Off by default since generated.h/cpp needs to be... well... generated for this to work
option(DYNAMIX_BUILD_PERF "DynaMix: build performance tests (requires manual code generation step)" OFF)

# Off by default since files need to be generated for this to work
option(DYNAMIX_BUILD_COMPILER_PERF "DynaMix: build compilation performance tests (requires manual code generation step)" OFF)

# config

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # so executables are next to the dll

# sources

set(dynamix_sources) # list to be filled with the source files for the project

set(dynamix_include ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(inc_path ${dynamix_include}/dynamix)
set(src_path ${CMAKE_CURRENT_SOURCE_DIR}/src)

src_group(public dynamix_sources
    ${inc_path}/allocators.hpp
    ${inc_path}/assert.hpp
    ${inc_path}/combinators.hpp
    ${inc_path}/common_mutation_rules.hpp
    ${inc_path}/config.hpp
    ${inc_path}/domain.hpp
    ${inc_path}/dynamix.hpp
    ${inc_path}/exception.hpp
    ${inc_path}/feature.hpp
    ${inc_path}/feature_parser.hpp
    ${inc_path}/message.hpp
    ${inc_path}/message_macros.hpp
    ${inc_path}/mixin.hpp
    ${inc_path}/mixin_id.hpp
    ${inc_path}/mixin_collection.hpp
    ${inc_path}/mixin_type_info.hpp
    ${inc_path}/mutation_rule.hpp
    ${inc_path}/mutation_rule_id.hpp
    ${inc_path}/next_bidder.hpp
    ${inc_path}/object.hpp
    ${inc_path}/object_mutator.hpp
    ${inc_path}/object_type_info.hpp
    ${inc_path}/object_type_mutation.hpp
    ${inc_path}/object_type_template.hpp
    ${inc_path}/preprocessor.hpp
    ${inc_path}/same_type_mutator.hpp
    ${inc_path}/single_object_mutator.hpp
    ${inc_path}/version.hpp
)

src_group("public~gen" dynamix_sources
    ${inc_path}/gen/template_message_macros.hpp
    ${inc_path}/gen/legacy_message_macros.hpp
    ${inc_path}/gen/no_arity_message_macros.hpp
    ${inc_path}/gen/short_message_macros.hpp
    ${inc_path}/gen/split_message_macros.hpp
    ${inc_path}/gen/undef_message_macros.hpp
)

src_group("private" dynamix_sources
    ${src_path}/allocators.cpp
    ${src_path}/common_mutation_rules.cpp
    ${src_path}/domain.cpp
    ${src_path}/internal.hpp
    ${src_path}/mixin_collection.cpp
    ${src_path}/mixin_type_info.cpp
    ${src_path}/object.cpp
    ${src_path}/object_mutator.cpp
    ${src_path}/object_type_info.cpp
    ${src_path}/object_type_mutation.cpp
    ${src_path}/object_type_template.cpp
    ${src_path}/same_type_mutator.cpp
    ${src_path}/single_object_mutator.cpp
    ${src_path}/zero_memory.hpp
)

if(DYNAMIX_SHARED_LIB)
    add_library(dynamix SHARED ${dynamix_sources})
    target_compile_definitions(dynamix PUBLIC
        -DDYNAMIX_DYNLIB
    )
else()
    add_library(dynamix STATIC ${dynamix_sources})
endif()

if(DYNAMIX_CUSTOM_CONFIG_FILE)
    target_compile_definitions(dynamix PUBLIC
        -DDYNAMIX_CUSTOM_CONFIG_FILE=${DYNAMIX_CUSTOM_CONFIG_FILE}
    )
endif()

set_target_properties(dynamix PROPERTIES FOLDER dynamix)

target_compile_features(dynamix PUBLIC
    cxx_alias_templates
    cxx_auto_type
    cxx_decltype
    cxx_delegating_constructors
    cxx_final
    cxx_lambdas
    cxx_override
    cxx_range_for
    cxx_right_angle_brackets
    cxx_rvalue_references
    cxx_static_assert
)

target_include_directories(dynamix PUBLIC
    ${dynamix_include}
)

# resolve options

if(DYNAMIX_BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(DYNAMIX_BUILD_EXAMPLES)
    if(DYNAMIX_EXAMPLE_HELLO_WORLD)
        add_subdirectory(example/hello_world)
    endif()

    if(DYNAMIX_EXAMPLE_BASIC)
        add_subdirectory(example/basic)
    endif()

    # the multi dll example is only valid if DynaMix is a shared library
    if(DYNAMIX_SHARED_LIB AND DYNAMIX_EXAMPLE_MULTI_DLL)
        add_subdirectory(example/multi_dll)
    endif()
endif()

if(DYNAMIX_BUILD_SCRATCH)
    add_subdirectory(example/scratch)
endif()

if(DYNAMIX_BUILD_TUTORIALS)
    add_subdirectory(tutorial)
endif()

if(DYNAMIX_BUILD_PERF)
    add_subdirectory(perf)
endif()

if(DYNAMIX_BUILD_COMPILER_PERF)
    add_subdirectory(compiler/perf)
endif()
