# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(dynamix-v1compat DYNAMIX_V1COMPAT)
add_library(dynamix::v1compat ALIAS dynamix-v1compat)
target_include_directories(dynamix-v1compat INTERFACE code)

target_link_libraries(dynamix-v1compat PUBLIC
    dynamix::dynamix
)

option(DYNAMIX_V1COMPAT_STATIC "DynaMix-v1compat: build as static lib" OFF)
if(DYNAMIX_STATIC)
    # cannot have v1compat shared with dynamix static
    set(DYNAMIX_V1COMPAT_STATIC ON)
endif()
option(DYNAMIX_V1COMPAT_GENERATE_CODE "DynaMix-v1compat: generate message macros with cmake" ON)

set(genOutDir ${CMAKE_CURRENT_SOURCE_DIR}/code/dynamix/v1compat/gen)
set(generatedFiles
    ${genOutDir}/template_message_macros.ipp
    ${genOutDir}/arity_message_macros.ipp
    ${genOutDir}/no_arity_message_macros.ipp
    ${genOutDir}/short_message_macros.ipp
    ${genOutDir}/undef_message_macros.ipp
    ${genOutDir}/create_combinator_call.ipp
)

if(DYNAMIX_V1COMPAT_GENERATE_CODE)
    if(NOT DEFINED DYNAMIX_V1COMPAT_GENERATE_ARITY)
        set(DYNAMIX_V1COMPAT_GENERATE_ARITY 5)
    endif()
    add_custom_command(
        COMMENT "DynaMix: Generating v1compat message macros"
        OUTPUT
            ${generatedFiles}
        COMMAND
            ruby gen_message_macros.rb ${DYNAMIX_V1COMPAT_GENERATE_ARITY}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/gen
        DEPENDS
            gen/gen_message_macros.rb
            gen/arity_message_macros_template
            gen/message_macros_template
            gen/no_arity_message_macros_template
            gen/short_message_macros_template
            gen/undef_message_macros_template
            gen/create_combinator_call_template
    )
endif()

target_sources(dynamix-v1compat PRIVATE
    code/dynamix/v1compat/api.h
    code/dynamix/v1compat/combinators.hpp
    code/dynamix/v1compat/core.hpp
    code/dynamix/v1compat/declare_message.hpp
    code/dynamix/v1compat/declare_mixin.hpp
    code/dynamix/v1compat/define_message.hpp
    code/dynamix/v1compat/define_mixin.hpp
    code/dynamix/v1compat/domain.hpp
    code/dynamix/v1compat/domain.cpp
    code/dynamix/v1compat/mutate.hpp
    code/dynamix/v1compat/mutation_rule.hpp
    code/dynamix/v1compat/common_mutation_rules.hpp
    code/dynamix/v1compat/mutation_rules.cpp
    code/dynamix/v1compat/next_bidder.hpp
    code/dynamix/v1compat/object.hpp
    code/dynamix/v1compat/object.cpp
    code/dynamix/v1compat/type_class.hpp
    ${generatedFiles}
)
