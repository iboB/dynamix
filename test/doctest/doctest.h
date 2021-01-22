#pragma once
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING 1
#include <string>
#include <iostream>
#include "doctest_fwd.h"

#if !DYNAMIX_USE_EXCEPTIONS
#   undef CHECK_NOTHROW
#   define CHECK_NOTHROW(x) x
#endif

#if defined(DOCTEST_CONFIG_IMPLEMENT)
#   include "doctest_impl.h"
#endif