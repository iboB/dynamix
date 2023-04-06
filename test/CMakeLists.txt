# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/doctest-util@0.1.0)
CPMAddPackage(gh:ThrowTheSwitch/Unity@2.5.2)
set_target_properties(doctest PROPERTIES FOLDER test)
set_target_properties(doctest-main PROPERTIES FOLDER test)
set_target_properties(unity PROPERTIES FOLDER test)

add_library(doctest-unity STATIC
    doctest-unity-main.cpp
)
set_target_properties(doctest-unity PROPERTIES FOLDER test)
target_link_libraries(doctest-unity PUBLIC
    unity::framework
    doctest::headers
)

macro(dynamix_x_test test)
    icm_add_test(
        NAME dynamix-x-${test}
        TARGET test-dynamix-x-${test}
        LIBRARIES
            doctest-unity
            dynamix::dynamix
        SOURCES
            x-${test}.c
            x-${test}.cpp
        FOLDER test
    )
endmacro()

macro(dynamix_c_test test)
    icm_add_test(
        NAME dynamix-c-${test}
        TARGET test-dynamix-c-${test}
        LIBRARIES
            unity::framework
            dynamix::dynamix
        SOURCES
            ${ARGN}
        FOLDER test
    )
endmacro()

macro(dynamix_test test)
    add_doctest_lib_test(${test} dynamix ${ARGN})
endmacro()

macro(dynamix_mt_test test)
    add_doctest_lib_test(${test} dynamix
        ${ARGN}
        LIBRARIES ${CMAKE_THREAD_LIBS_INIT}
        LABELS tsan
    )
endmacro()

dynamix_x_test(sv)

dynamix_test(bits-id t-bits-id.cpp)
dynamix_test(alloc_util t-alloc_util.cpp)
dynamix_test(bits-make_from_tuple t-bits-make_from_tuple.cpp)

dynamix_test(feature_info_data t-feature_info_data.cpp)

dynamix_c_test(mixin_info_util t-mixin_info_util.c)
dynamix_test(mixin_info_util t-mixin_info_util.cpp)

dynamix_test(test_data t-test_data.cpp)

dynamix_test(domain-empty t-domain-empty.cpp)

dynamix_test(mixin_info_data t-mixin_info_data.cpp)

dynamix_test(domain-features t-domain-features.cpp)
dynamix_test(domain-mixins t-domain-mixins.cpp)
dynamix_test(type t-type.cpp)
dynamix_test(type_class t-type_class.cpp)
dynamix_test(type_mutation t-type_mutation.cpp)
dynamix_test(mutation_rule t-mutation_rule.cpp)

dynamix_test(object t-object.cpp)

dynamix_c_test(domain t-domain.c)
dynamix_c_test(type t-type.c)
dynamix_c_test(object t-object.c)

dynamix_test(define_mixin t-define_mixin.cpp)

dynamix_test(msg t-msg.cpp)

dynamix_test(danger t-danger.cpp)

if(DYNAMIX_SHARED)
    add_subdirectory(app-plugin)
endif()

find_package(Threads)
if(Threads_FOUND)
    dynamix_mt_test(domain-mt t-domain-mt.cpp)

    dynamix_mt_test(fuzz-features fuzz-features.cpp)
    dynamix_mt_test(fuzz-mixins fuzz-mixins.cpp)
endif()

if(DYNAMIX_BUILD_V1COMPAT)
    add_subdirectory(v1compat)
endif()
