macro(dynamix_v1compat_test test)
    add_doctest_lib_test(${test} dynamix-v1compat ${ARGN})
endmacro()

dynamix_v1compat_test(core tv1-core.cpp)
