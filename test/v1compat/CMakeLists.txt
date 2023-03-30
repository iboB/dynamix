macro(dynamix_v1compat_test test)
    add_doctest_lib_test(${test} dynamix-v1compat ${ARGN})
endmacro()

dynamix_v1compat_test(core tv1-core.cpp)
dynamix_v1compat_test(arg_copy tv1-arg_copy.cpp)
dynamix_v1compat_test(alignment tv1-alignment.cpp)
dynamix_v1compat_test(by_string tv1-by_string.cpp)
dynamix_v1compat_test(bind_feature tv1-bind_feature.cpp)
