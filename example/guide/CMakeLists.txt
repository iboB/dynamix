# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
# Companion examples for the C++ guide in doc/cpp
# They are also added as tests, so that they're verified to build and run

macro(dynamix_guide_example name)
    add_executable(dynamix-guide-${name} ${name}.cpp)
    target_link_libraries(dynamix-guide-${name} dynamix::dynamix)
    set_target_properties(dynamix-guide-${name} PROPERTIES FOLDER example/guide)
    if(DYNAMIX_BUILD_TESTS)
        add_test(NAME dynamix-guide-${name} COMMAND dynamix-guide-${name})
    endif()
endmacro()

dynamix_guide_example(messages)
dynamix_guide_example(mixins)
dynamix_guide_example(mutations)
dynamix_guide_example(domains)
dynamix_guide_example(objects)
