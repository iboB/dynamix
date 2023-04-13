// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-virtual.hpp"
#include "bmm-common.hpp"

base_op::~base_op() = default;

#define DEFINE_OP(name, code) \
    struct vop_##name : base_op { \
        uint32_t m_val_a, m_val_b; \
        vop_##name(uint32_t a, uint32_t b) : m_val_a(a), m_val_b(b) {} \
        void accumulate(uint64_t& acc, uint32_t arg) const override { \
            code; \
        } \
    };

ITERATE_OPS(DEFINE_OP)

#define CASE_OP(name, code) \
    case name: return std::make_unique<vop_##name>(a, b);

std::unique_ptr<base_op> fop_from_op(op_id op, uint32_t a, uint32_t b) {
    switch (op) {
    ITERATE_OPS(CASE_OP)
    default: return {};
    }
}

virt_accumulator make_virt_accumulator(std::minstd_rand& rnd) {
    auto combo = get_ops_combo(rnd);

    virt_accumulator ret;
    for (auto o : combo) {
        ret.ops.push_back(fop_from_op(o.op, o.a, o.b));
    }
    return ret;
}
