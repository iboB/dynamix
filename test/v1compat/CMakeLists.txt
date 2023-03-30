macro(dynamix_v1compat_test test)
    add_doctest_lib_test(${test} dynamix-v1compat ${ARGN})
endmacro()

dynamix_v1compat_test(core tv1-core.cpp)
dynamix_v1compat_test(arg_copy tv1-arg_copy.cpp)
dynamix_v1compat_test(alignment tv1-alignment.cpp)
