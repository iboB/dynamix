// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

// optionally don't include any message macros and let the user decide which
// set they're going to use for each message
#if !defined(DYNAMIX_NO_MESSAGE_MACROS)
// include the generated macros
// choose definition header
// making this choice DOES NOT require you to rebuild the library
// these headers are purely user facing
#if defined(DYNAMIX_USE_LEGACY_MESSAGE_MACROS)
// this file contains the old-style macros which have a lot of the calling code
// in the macros itself. They make it a bit harder to step into messages when
// debugging but in some cases with gcc and clang compile much faster
#   include "declare_message_legacy.hpp"
#else
// these are the new-style message macros only a single step into is needed when
// debugging in order to go to debuggable c++ code
// however they may be much slower to compile on gcc and clang
// this is generally the recommended header, but users are encouraged to test
// their compilation times with gcc and clang with the other header as well
#   include "declare_message.hpp"
#endif
#endif
