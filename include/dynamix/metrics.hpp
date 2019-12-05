// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once


/**
 * \file
 * Defines types for additional metrics
 */

#if DYNAMIX_THREAD_SAFE_MUTATIONS
#include <atomic>
namespace dynamix
{
using metric = std::atomic<size_t>;
}
#else
namespace dynamix
{
using metric = size_t;
}
#endif
