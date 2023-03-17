# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(dynamix-testlib TESTLIB)
target_include_directories(dynamix-testlib INTERFACE ..)

add_library(dynamix::testlib ALIAS dynamix-testlib)

CPMAddPackage(gh:iboB/splat@1.2.2)

target_link_libraries(dynamix-testlib PUBLIC
    splat::splat
    dynamix::dynamix
)

target_sources(dynamix-testlib PRIVATE
    api.h
    domain_tag.hpp
    impl.cpp
)