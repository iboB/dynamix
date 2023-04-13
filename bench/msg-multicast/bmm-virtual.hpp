// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <random>

struct base_op {
    virtual ~base_op();
    virtual void accumulate(uint64_t& acc, uint32_t arg) const = 0;
};

struct virt_accumulator {
    std::vector<std::unique_ptr<base_op>> ops;
    uint64_t accumulate(uint32_t arg) const {
        uint64_t acc = 0;
        for (auto& op : ops) {
            op->accumulate(acc, arg);
        }
        return acc;
    }
};

virt_accumulator make_virt_accumulator(std::minstd_rand& rnd);

