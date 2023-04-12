// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <random>

struct func_accumulator {
    using func = std::function<void(uint64_t&, uint32_t)>;
    std::vector<func> ops;
    uint64_t accumulate(uint32_t val) const {
        uint64_t acc = 0;
        for (auto& f : ops) {
            f(acc, val);
        }
        return acc;
    }
};

func_accumulator make_func_accumulator(std::minstd_rand& rnd);
