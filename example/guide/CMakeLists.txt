# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-example-domains e-domains.cpp)
target_link_libraries(dynamix-example-domains dynamix::dynamix)
set_target_properties(dynamix-example-domains PROPERTIES FOLDER example)
