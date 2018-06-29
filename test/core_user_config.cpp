// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

// run the code tests with custom user configuratble definitions

#if defined(DYNAMIX_USE_LEGACY_MESSAGE_MACROS)
#   undef DYNAMIX_USE_LEGACY_MESSAGE_MACROS
#else
#   define DYNAMIX_USE_LEGACY_MESSAGE_MACROS
#endif

#include "core.cpp"
