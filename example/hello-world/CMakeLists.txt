# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-example-hello-world hello-world.cpp)
target_link_libraries(dynamix-example-hello-world dynamix::dynamix)
set_target_properties(dynamix-example-hello-world PROPERTIES FOLDER example)
