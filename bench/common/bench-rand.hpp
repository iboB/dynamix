// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <itlib/span.hpp>
#include <random>
#include <numeric>
#include <cassert>

// deterministic random utilities for benchmarks
// we don't use std::shuffle, std::uniform_int_distribution, or std::unordered_set iteration order,
// as they have different implementations in different standard libraries,
// and the same seed would produce different data on different platforms
namespace dbench {

using rng_t = std::minstd_rand;

template <typename T>
void shuffle(itlib::span<T> span, rng_t& rng) {
    // Fisher-Yates
    for (size_t i = span.size(); i > 1; --i) {
        auto j = size_t(rng() % i);
        std::swap(span[i - 1], span[j]);
    }
}

template <typename T>
void shuffle(std::vector<T>& vec, rng_t& rng) {
    return shuffle(itlib::span<T>(vec), rng);
}

// pick `count` unique values in [0; range) in random order
// requires count <= range
inline std::vector<uint32_t> pick_unique(uint32_t count, uint32_t range, rng_t& rng) {
    assert(count <= range);
    std::vector<uint32_t> ret(range);
    std::iota(ret.begin(), ret.end(), 0);

    // Fisher-Yates, but partially for the first `count` elements
    for (uint32_t i = 0; i < count; ++i) {
        auto j = i + uint32_t(rng() % (range - i));
        std::swap(ret[i], ret[j]);
    }
    ret.resize(count);
    return ret;
}

// benchmark user data values to select the access order
enum access : uintptr_t {
    access_sequential = 0,
    access_shuffled = 1,
};

// the order in which to access n elements in a benchmark:
// sequential (0, 1, 2, ...) or shuffled
inline std::vector<uint32_t> make_access_order(size_t n, access a, rng_t& rng) {
    std::vector<uint32_t> ret(n);
    std::iota(ret.begin(), ret.end(), 0);
    if (a == access_shuffled) shuffle(ret, rng);
    return ret;
}

} // namespace dbench
