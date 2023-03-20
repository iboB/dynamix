# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(icm_add_lib)

icm_add_lib(dynamix DYNAMIX)
add_library(dynamix::dynamix ALIAS dynamix)
target_include_directories(dynamix INTERFACE .)

CPMAddPackage("gh:iboB/itlib@1.8.3b#c426d5d27d2121eebacd37de0d2df4ffdf930562")
CPMAddPackage("gh:iboB/splat@1.3.0a#c6e8650c34105e741e3a887f1a944976a9f58261")

target_link_libraries(dynamix PUBLIC
    splat::splat
    itlib::itlib
)

target_sources(dynamix PRIVATE
    dynamix/domain.cpp
    dynamix/object.cpp
    "dynamix/mixin_allocator-c.cpp"
    dynamix/type.cpp
    dynamix/exception.cpp
    dynamix/type_class.cpp
    dynamix/type_mutation.cpp
    dynamix/type_mutation-c.cpp
    dynamix/object_mutation.cpp
    dynamix/object_mutate_ops.cpp

    dynamix/compat/pmr/memory_resource.cpp

    "dynamix/domain-c.cpp"
 "dnmx/object_handle.h" "dnmx/basic_object.h" "dnmx/object_mixin_data.h" "dynamix/type-c.cpp" "dnmx/object.h" "dynamix/object-c.cpp" "dnmx/mixin_common_funcs.h" "dnmx/bits/noexcept.h" "dnmx/mixin_common_funcs.c" "dnmx/mixin_info_util.h" "dynamix/mixin_info_util.hpp"  "dnmx/mutation_rule_info.h"  "dnmx/msg/msg_macros.h" "dynamix/mixin_info_data.cpp" "dynamix/define_domain.hpp")
