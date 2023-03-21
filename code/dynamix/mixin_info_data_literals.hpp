// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>
#include <optional>

namespace dynamix::util {
struct builder_perks {
    std::optional<int32_t> bid;
    std::optional<int32_t> priority;
    builder_perks operator-() const {
        // C++23 optional::transform
        builder_perks ret = *this;
        if (ret.bid) *ret.bid = -*ret.bid;
        if (ret.priority) *ret.priority = -*ret.priority;
        return ret;
    }
};

namespace builder_literals {
inline builder_perks feature_bid(int32_t b) { return {b, std::nullopt}; }
inline builder_perks order_priority(int32_t p) { return {std::nullopt, p}; }
inline builder_perks feature_bid_priority(int32_t b, int32_t p) { return {b, p}; }

inline builder_perks operator ""_bid(unsigned long long b) { return feature_bid(int32_t(b)); }
inline builder_perks operator ""_prio(unsigned long long p) { return order_priority(int32_t(p)); }
}
}