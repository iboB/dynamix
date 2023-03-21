// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmu-std_func.hpp"
#include "pi.hpp"

area_func get_square_func(int side) {
    return [=](int mod) {
        return side * side * mod;
    };
}

area_func get_rect_func(int a, int b) {
    return [=](int mod) {
        return a * b * mod;
    };
}

area_func get_circle_func(int radius) {
    return [=](int mod) {
        double r3 = radius * radius * pi;
        return int(r3) * mod;
    };
}

area_func get_triangle_func(int a, int ha) {
    return [=](int mod) {
        return mod * a * ha / 2;
    };
}

