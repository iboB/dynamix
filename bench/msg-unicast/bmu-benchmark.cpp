// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmu-msg.hpp"
#include "bmu-std_func.hpp"
#include "bmu-virtual.hpp"

#include <picobench/picobench.hpp>

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

#include <memory>
#include <random>

constexpr uint32_t seed = 42;

void virt(picobench::state& pb) {
    std::minstd_rand rng(seed);
    auto rnd = [&]() {
        return rng() % 10;
    };

    std::vector<std::unique_ptr<shape>> shapes(size_t(pb.iterations()));
    for (auto& s : shapes) {
        auto type = rng() % 4;
        int a = rnd();
        int b = rnd();
        switch (type) {
        case 0:
            s.reset(new square(a));
            break;
        case 1:
            s.reset(new rect(a, b));
            break;
        case 2:
            s.reset(new circle(a));
            break;
        case 3:
            s.reset(new triangle(a, b));
            break;
        }
    }

    uintptr_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto& s : shapes) {
        sum += s->area(rnd());
    }
    pb.set_result(sum);
}

PICOBENCH(virt).baseline();

void std_func(picobench::state& pb) {
    std::minstd_rand rng(seed);
    auto rnd = [&]() {
        return rng() % 10;
    };

    std::vector<area_func> shapes(size_t(pb.iterations()));
    for (auto& s : shapes) {
        auto type = rng() % 4;
        int a = rnd();
        int b = rnd();
        switch (type) {
        case 0:
            s = get_square_func(a);
            break;
        case 1:
            s = get_rect_func(a, b);
            break;
        case 2:
            s = get_circle_func(a);
            break;
        case 3:
            s = get_triangle_func(a, b);
            break;
        }
    }

    uintptr_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto& s : shapes) {
        sum += s(rnd());
    }
    pb.set_result(sum);
}

PICOBENCH(std_func);

struct bench_obj : public dynamix::object {
    bench_obj() : dynamix::object(dynamix::g::get_domain<bench>()) {}
};

void dynamix_msg(picobench::state& pb) {
    std::minstd_rand rng(seed);
    auto rnd = [&]() {
        return rng() % 10;
    };

    std::vector<bench_obj> shapes(size_t(pb.iterations()));
    for (auto& s : shapes) {
        auto type = rng() % 4;
        int a = rnd();
        int b = rnd();
        switch (type) {
        case 0:
            dynamix::mutate(s, mixin_square(a));
            break;
        case 1:
            dynamix::mutate(s, mixin_rect(a, b));
            break;
        case 2:
            dynamix::mutate(s, mixin_circle(a));
            break;
        case 3:
            dynamix::mutate(s, mixin_triangle(a, b));
            break;
        }
    }

    uintptr_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto& s : shapes) {
        sum += area(s, rnd());
    }
    pb.set_result(sum);
}

PICOBENCH(dynamix_msg);
