// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-virtual.hpp"

base_op::~base_op() = default;

#define DEFINE_OP(name, code) \
    struct op_##name : base_op { \
        uint32_t m_val_a, m_val_b; \
        op_##name(uint32_t a, uint32_t b) : m_val_a(a), m_val_b(b) {} \
        void accumulate(uint64_t& acc, uint32_t arg) const override { \
            code; \
        } \
    };