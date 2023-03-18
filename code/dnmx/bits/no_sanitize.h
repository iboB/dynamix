// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#if defined(__GNUC__)
#define I_DYNAMIX_NO_SANITIZE(...)  __attribute__((no_sanitize(__VA_ARGS__)))
#else
#define I_DYNAMIX_NO_SANITIZE(...)
#endif
