// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include "dynamix/mixin_type_info.hpp"

#if DYNAMIX_USE_TYPEID && defined(__GNUC__)
#   include <cstdlib>
#   include <cxxabi.h>
static int cxa_demangle_status;
#endif

namespace dynamix
{
namespace internal
{
#if DYNAMIX_USE_TYPEID // using typeid: tested for msvc, gcc and clang
const char* get_mixin_name_from_typeid(const char* typeid_name)
{
#if defined(__GNUC__) // __GNUC__ is defined with clang
    // use cxxabi to unmangle the gcc typeid name
    return abi::__cxa_demangle(typeid_name, nullptr, nullptr, &cxa_demangle_status);
#elif defined(_MSC_VER)
    // msvc typeid names are "class x" or "struct x" instead of "x",
    // so we just offset the pointer appropriately
    while (*typeid_name++ != 32);
    return typeid_name;
#else
#    error "getting typenames with typeid hasn't been tested on compilers other than gcc, clang, and msvc"
#endif
}

#if defined(__GNUC__)
void free_mixin_name_from_typeid(const char* typeid_name)
{
    // strings obtained by cxa_demangle must be freed
    free(const_cast<void*>(static_cast<const void*>(typeid_name)));
}
#endif
#endif // DYNAMIX_USE_TYPEID

}
}
