# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(dynamix-example-hello-c-world hello-world.c)
target_link_libraries(dynamix-example-hello-c-world dynamix::dynamix)
set_target_properties(dynamix-example-hello-c-world PROPERTIES FOLDER example)
