// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmu-msg.hpp"
#include "bmu-std_func.hpp"
#include "bmu-virtual.hpp"

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

#include <memory>

#include <picobench/picobench.hpp>
#include <bench-main.hpp>
#include <bench-rand.hpp>

constexpr uint32_t seed = 42;

// all data for a benchmark is generated before the measured part
// so that random number generation does not contribute to the measurement
struct test_data {
    struct shape_desc {
        uint32_t type;
        int a, b;
    };
    std::vector<shape_desc> shapes;

    // order in which to access the objects
    // in the shuffled case consecutive calls are on objects which are not adjacent in memory
    std::vector<uint32_t> order;

    // args[i] is the argument for the i-th call (not for the i-th object)
    // this way args are accessed sequentially in all cases
    std::vector<int> args;

    explicit test_data(const picobench::state& pb) {
        dbench::rng_t rng(seed);

        const auto n = size_t(pb.iterations());

        shapes.resize(n);
        for (auto& s : shapes) {
            s.type = rng() % 4;
            s.a = int(rng() % 10);
            s.b = int(rng() % 10);
        }

        args.resize(n);
        for (auto& a : args) {
            a = int(rng() % 10);
        }

        const auto access_type = dbench::access(pb.benchmark_user_data());
        order = dbench::make_access_order(n, access_type, rng);
    }
};

void virt(picobench::state& pb) {
    const test_data data(pb);

    std::vector<std::unique_ptr<shape>> shapes;
    shapes.reserve(data.shapes.size());
    for (auto& d : data.shapes) {
        switch (d.type) {
        case 0: shapes.emplace_back(new square(d.a)); break;
        case 1: shapes.emplace_back(new rect(d.a, d.b)); break;
        case 2: shapes.emplace_back(new circle(d.a)); break;
        case 3: shapes.emplace_back(new triangle(d.a, d.b)); break;
        }
    }

    uintptr_t sum = 0;
    for (auto i : pb) {
        sum += shapes[data.order[i]]->area(data.args[i]);
    }
    pb.set_result(sum);
}

void std_func(picobench::state& pb) {
    const test_data data(pb);

    std::vector<area_func> shapes;
    shapes.reserve(data.shapes.size());
    for (auto& d : data.shapes) {
        switch (d.type) {
        case 0: shapes.push_back(get_square_func(d.a)); break;
        case 1: shapes.push_back(get_rect_func(d.a, d.b)); break;
        case 2: shapes.push_back(get_circle_func(d.a)); break;
        case 3: shapes.push_back(get_triangle_func(d.a, d.b)); break;
        }
    }

    uintptr_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto i : pb) {
        sum += shapes[data.order[i]](data.args[i]);
    }
    pb.set_result(sum);
}

struct bench_obj : public dynamix::object {
    bench_obj() : dynamix::object(dynamix::g::get_domain<bench>()) {}
};

void dynamix_msg(picobench::state& pb) {
    const test_data data(pb);

    std::vector<bench_obj> shapes(data.shapes.size());
    for (size_t i = 0; i < shapes.size(); ++i) {
        auto& d = data.shapes[i];
        auto& s = shapes[i];
        switch (d.type) {
        case 0: dynamix::mutate(s, mixin_square(d.a)); break;
        case 1: dynamix::mutate(s, mixin_rect(d.a, d.b)); break;
        case 2: dynamix::mutate(s, mixin_circle(d.a)); break;
        case 3: dynamix::mutate(s, mixin_triangle(d.a, d.b)); break;
        }
    }

    uintptr_t sum = 0;
    picobench::scope benchmark(pb);
    for (auto i : pb) {
        sum += area(shapes[data.order[i]], data.args[i]);
    }
    pb.set_result(sum);
}

// objects are accessed in the order in which they were created
// since they are allocated one after the other, the hardware prefetcher hides most cache misses
PICOBENCH_SUITE("unicast: sequential access");
PICOBENCH(virt).user_data(dbench::access_sequential).baseline();
PICOBENCH(std_func).user_data(dbench::access_sequential);
PICOBENCH(dynamix_msg).user_data(dbench::access_sequential);

// objects are accessed in random order
// this is closer to real-world code where objects are created at different times and referenced from elsewhere
PICOBENCH_SUITE("unicast: shuffled access");
PICOBENCH(virt).user_data(dbench::access_shuffled).baseline();
PICOBENCH(std_func).user_data(dbench::access_shuffled);
PICOBENCH(dynamix_msg).user_data(dbench::access_shuffled);

int main(int argc, char* argv[]) {
    return dbench::run_bench(argc, argv, {1024, 16384, 65536});
}
