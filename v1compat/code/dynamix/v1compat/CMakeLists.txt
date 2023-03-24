# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(dynamix-v1compat DYNAMIX_V1_COMPAT)
add_library(dynamix::v1compat ALIAS dynamix-v1compat)
target_include_directories(dynamix-v1compat INTERFACE ../..)

target_link_libraries(dynamix-v1compat PUBLIC
    dynamix::dynamix
)

target_sources(dynamix-v1compat PRIVATE
    "v1domain.cpp"
)
