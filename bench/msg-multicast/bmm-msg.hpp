// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/object.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <cstdint>
#include <random>

class dobject;
DYNAMIX_DECLARE_MSG(accumulate_msg, accumulate, void, (const dobject&, uint64_t&, uint32_t));

class dobject : public dynamix::object {
public:
    using dynamix::object::object;
    uint64_t accumulate(uint32_t arg) const {
        uint64_t acc = 0;
        ::accumulate(*this, acc, arg);
        return acc;
    }
};

dobject make_msg_accumulator(std::minstd_rand& rnd);
