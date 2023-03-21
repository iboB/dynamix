// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

struct shape {
    virtual ~shape();
    virtual int area(int mod) const = 0;
};

struct square final : public shape {
    square(int side) : m_side(side) {}
    virtual int area(int mod) const override;
    int m_side;
};

struct rect final : public shape {
    rect(int a, int b) : m_a(a), m_b(b) {}
    virtual int area(int mod) const override;
    int m_a, m_b;
};

struct circle final : public shape {
    circle(int radius) : m_radius(radius) {}
    virtual int area(int mod) const override;
    int m_radius;
};

struct triangle final : public shape {
    triangle(int a, int ha) : m_a(a), m_ha(ha) {}
    virtual int area(int mod) const override;
    int m_a, m_ha;
};
