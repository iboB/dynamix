# MaiBo
# Copyright (c) 2015 Borislav Stanimirov
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
# src_group
#
# Defines a group of source files, while also appending them to a list
# (to be used with add_executable or add_library)
# Args:
# GROUP_NAME - name of the group. Subroups are separated by "~"
# SRC_LIST - list of sources to append to
# ... the other arguments are a list of files
#
# Example Usage
# set(mySources)
# src_group("Group~Subgroup" mySources file1 file2 ... fileN)
# add_executable(myexe mySources)
#
macro(src_group GROUP_NAME SRC_LIST)
    # In order to work CMake's source_group macro requiers backslashes between
    # groups and subgroups. However this leads to escape hell. That's why we
    # use tildes and then replace them with backslashes at the very end
    set(DEMANGLED_GROUP)
    string(REPLACE "~" "\\\\" DEMANGLED_GROUP ${GROUP_NAME})

    source_group(${DEMANGLED_GROUP} FILES ${ARGN})

    foreach(filename ${ARGN})
        list(APPEND ${SRC_LIST} ${filename})
    endforeach()
endmacro(src_group)
