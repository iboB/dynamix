# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
macro(dynamix_example name)
	add_executable(dynamix-example-${name} ${ARGN})
	target_link_libraries(dynamix-example-${name} dynamix::dynamix)
	set_target_properties(dynamix-example-${name} PROPERTIES FOLDER example)
endmacro()

dynamix_example(hello-world hello-world.cpp)
dynamix_example(hello-c-world hello-world.c)

add_subdirectory(guide)
