// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Optional header file, which contains library version info.
 */

#define DYNAMIX_VERSION_MAJOR 1
#define DYNAMIX_VERSION_MINOR 2
#define DYNAMIX_VERSION_SUB_MINOR 2

/// The library's version.
///
/// It is composed of `major*10000 + minor*100 + subminor`.
/// For example, version 2.31.11 would be 23111.
#define DYNAMIX_VERSION (DYNAMIX_VERSION_MAJOR*10000 + DYNAMIX_VERSION_MINOR*100 + DYNAMIX_VERSION_SUB_MINOR)
