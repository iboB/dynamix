// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#define DYNAMIX_MAX_MIXINS 256
#define DYNAMIX_MAX_MESSAGES 512
#define DYNAMIX_USE_EXCEPTIONS 0
#define DYNAMIX_OBJECT_IMPLICIT_COPY 1
#define DYNAMIX_THREAD_SAFE_MUTATIONS 0
#define DYNAMIX_ADDITIONAL_METRICS 0

// the following don't affect the build of the library but we'll just
// use the opportunity to run tests with them
#define DYNAMIX_NO_MSG_THROW
#define DYNAMIX_USE_LEGACY_MESSAGE_MACROS
