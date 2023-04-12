// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

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

enum ops {
    ITERATE_OPS(ENUM)
    ops_count
};
