// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmu-virtual.hpp"
#include "pi.hpp"

shape::~shape() = default;

int square::area(int mod) const {
    return m_side * m_side * mod;
}

int rect::area(int mod) const {
    return m_a * m_b * mod;
}

int circle::area(int mod) const {
    double r3 = m_radius * m_radius * pi;
    return int(r3) * mod;
}

int triangle::area(int mod) const {
    return mod * m_a * m_ha / 2;
}
