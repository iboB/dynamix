// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-std_func.hpp"
#include "bmm-common.hpp"

#define DEFINE_OP(name, code) \
    void fop_##name(uint32_t m_val_a, [[maybe_unused]] uint32_t m_val_b, uint64_t& acc, uint32_t arg) { \
        code; \
    } \

ITERATE_OPS(DEFINE_OP)

#define CASE_OP(name, code) \
    case name: return [=](uint64_t& acc, uint32_t arg) { fop_##name(a, b, acc, arg); };

func_accumulator::func vop_from_op(op_id op, uint32_t a, uint32_t b) {
    switch (op) {
        ITERATE_OPS(CASE_OP)
    default: return {};
    }
}

func_accumulator make_func_accumulator(std::minstd_rand& rnd) {
    auto combo = get_ops_combo(rnd);

    func_accumulator ret;
    for (auto o : combo) {
        ret.ops.push_back(vop_from_op(o.op, o.a, o.b));
    }
    return ret;
}

