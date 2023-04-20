// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <typeinfo>
#include <string>
#include <itlib/strutil.hpp>

#if defined(__GNUC__)
#   include <cstdlib>
#   include <cxxabi.h>
#endif

namespace dynamix::util {
std::string get_type_name_from_typeid(const char* typeid_name) {
#if defined(__GNUC__) // __GNUC__ is defined with clang
    // use cxxabi to unmangle the gcc typeid name
    int cxa_demangle_status = 0;
    auto str = abi::__cxa_demangle(typeid_name, nullptr, nullptr, &cxa_demangle_status);
    std::string ret = str;
    free(typeid_name);
    return ret;
#elif defined(_MSC_VER)
    std::string_view sv_name = typeid_name;

    // msvc typeid names are "class x" or "struct x" instead of "x",
    // so we just offset the pointer appropriately
    // C++20 use std::string_view::starts_with
    if (itlib::starts_with(sv_name, "enum ")) return typeid_name + 5;
    if (itlib::starts_with(sv_name, "class ")) return typeid_name + 6;
    if (itlib::starts_with(sv_name, "struct ")) return typeid_name + 7;

    return typeid_name;
#else
#    error "getting typenames with typeid hasn't been tested on compilers other than gcc, clang, and msvc"
#endif
}

template <typename T>
std::string get_type_name_from_typeid() {
    return get_type_name_from_typeid(typeid(T).name());
}
}
