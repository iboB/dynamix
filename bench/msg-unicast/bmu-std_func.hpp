// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <functional>

using area_func = std::function<int(int)>;

area_func get_square_func(int side);
area_func get_rect_func(int a, int b);
area_func get_circle_func(int radius);
area_func get_triangle_func(int a, int ha);
