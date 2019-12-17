// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"

#include <cstddef>

/**
 * \file
 * Defines types for additional metrics
 */

#if DYNAMIX_THREAD_SAFE_MUTATIONS
#include <atomic>
namespace dynamix
{
struct metric
{
    metric(size_t v) : value(v) {}

    operator size_t() const
    {
        return value.load(std::memory_order_relaxed);
    }

    metric& operator++()
    {
        value.fetch_add(1, std::memory_order_relaxed);
        return *this;
    }

    metric& operator--()
    {
        value.fetch_sub(1, std::memory_order_relaxed);
        return *this;
    }

    mutable std::atomic<size_t> value;
};
}
#else
namespace dynamix
{
using metric = size_t;
}
#endif
