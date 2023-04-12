// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>
#include <vector>
#include <random>

#define ITERATE_OPS(MACRO) \
    MACRO(plus_plus, acc += arg + m_val_a) \
    MACRO(minus_minus, acc -= arg + m_val_a) \
    MACRO(plus_mul, acc += arg * m_val_a) \
    MACRO(plus_div, acc += arg / m_val_a) \
    MACRO(minus_mul, acc -= arg * m_val_a) \
    MACRO(plus_plus_plus, acc += arg + m_val_a + m_val_b) \
    MACRO(minus_minus_minus, acc -= arg + m_val_a + m_val_b) \
    MACRO(plus_mul_plus, acc += arg * m_val_a + m_val_b) \
    MACRO(plus_mul_minus, acc += arg * m_val_a - m_val_b) \
    MACRO(plus_plus_mul, acc += arg + m_val_a * m_val_b) \
    MACRO(plus_div_plus, acc += arg / m_val_a + m_val_b) \


#define ENUM(name, code) name,

enum op_id : uint32_t {
    ITERATE_OPS(ENUM)
    ops_count
};

struct op_data {
    op_id op;
    uint32_t a;
    uint32_t b;
};
std::vector<op_data> get_ops_combo(std::minstd_rand& rnd);
