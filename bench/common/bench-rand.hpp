// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>
#include <cstddef>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

// deterministic random utilities for benchmarks
// we don't use std::shuffle, std::uniform_int_distribution, or std::unordered_set iteration order,
// as they have different implementations in different standard libraries,
// and the same seed would produce different data on different platforms
namespace dbench {

using rng_t = std::minstd_rand;

// Fisher-Yates shuffle
template <typename T>
void shuffle(std::vector<T>& vec, rng_t& rng) {
    for (size_t i = vec.size(); i > 1; --i) {
        auto j = size_t(rng() % i);
        std::swap(vec[i - 1], vec[j]);
    }
}

// pick `count` unique values in [0; range) in random order
// requires count <= range
inline std::vector<uint32_t> pick_unique(uint32_t count, uint32_t range, rng_t& rng) {
    std::vector<uint32_t> all(range);
    std::iota(all.begin(), all.end(), 0);
    // partial Fisher-Yates
    for (uint32_t i = 0; i < count; ++i) {
        auto j = i + uint32_t(rng() % (range - i));
        std::swap(all[i], all[j]);
    }
    all.resize(count);
    return all;
}

// the order in which to access n elements in a benchmark:
// sequential (0, 1, 2, ...) or shuffled
inline std::vector<uint32_t> make_access_order(size_t n, bool shuffled, rng_t& rng) {
    std::vector<uint32_t> ret(n);
    std::iota(ret.begin(), ret.end(), 0);
    if (shuffled) shuffle(ret, rng);
    return ret;
}

// benchmark user data values to select the access order
enum access : uintptr_t {
    sequential = 0,
    shuffled = 1,
};

} // namespace dbench
