// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bth-common.hpp"

#include <dynamix/type.hpp>

#include <bench-rand.hpp>
#include <bench-mixin-set.hpp>

#include <atomic>
#include <set>
#include <thread>

#define PICOBENCH_IMPLEMENT
#include <bench-main.hpp>

// This benchmark measures how DynaMix operations scale with multiple threads.
// The work (a fixed number of operations) is split between threads and the wall time is measured,
// so with perfect scaling the time per operation of the multi-threaded benchmarks would be
// 1/NUM_THREADS of the single-threaded ones.
// The results are only meaningful on machines with at least NUM_THREADS cores.

constexpr unsigned NUM_THREADS = 4;

constexpr uint32_t seed = 77;

// split [0; n) into num_threads chunks and run func(begin, end) for each chunk in a separate thread
// the threads are created and set up before the measured part and wait for a signal to start
// returns the sum of the results of func
template <typename Func>
uintptr_t run_parallel(picobench::state& pb, unsigned num_threads, size_t n, Func func) {
    std::atomic<unsigned> ready = 0;
    std::atomic<bool> go = false;
    std::vector<uintptr_t> results(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (unsigned t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            // the first allocation in a thread may be expensive (the standard library may set up
            // thread-local allocation structures), so do it before the measured part
            delete new int(0);
            ready.fetch_add(1, std::memory_order_release);
            while (!go.load(std::memory_order_acquire)) std::this_thread::yield();
            const auto begin = n * t / num_threads;
            const auto end = n * (t + 1) / num_threads;
            results[t] = func(begin, end);
        });
    }

    while (ready.load(std::memory_order_acquire) != num_threads) std::this_thread::yield();

    {
        dbench::scope benchmark(pb);
        go.store(true, std::memory_order_release);
        for (auto& th : threads) th.join();
    }

    uintptr_t ret = 0;
    for (auto r : results) ret += r;
    return ret;
}

struct test_data {
    std::vector<int> values;
    std::vector<int> args;
    explicit test_data(const picobench::state& pb) {
        dbench::rng_t rng(seed);
        const auto n = size_t(pb.iterations());
        values.resize(n);
        for (auto& v : values) v = int(rng() % 10);
        args.resize(n);
        for (auto& a : args) a = int(rng() % 10);
    }
};

//////////////////////////////////////////////////////////////////////////////
// message calls
// the objects are shared between threads (read-only)

namespace calls {

void virt_benchmark(picobench::state& pb, unsigned num_threads) {
    const test_data data(pb);
    std::vector<entity> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_entity(v));

    auto sum = run_parallel(pb, num_threads, objs.size(), [&](size_t begin, size_t end) {
        uintptr_t s = 0;
        for (size_t i = begin; i < end; ++i) {
            s += objs[i].get_value(data.args[i]);
        }
        return s;
    });
    pb.set_result(sum);
}

void dynamix_benchmark(picobench::state& pb, unsigned num_threads) {
    const test_data data(pb);
    std::vector<dynamix::object> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_obj(v));

    auto sum = run_parallel(pb, num_threads, objs.size(), [&](size_t begin, size_t end) {
        uintptr_t s = 0;
        for (size_t i = begin; i < end; ++i) {
            s += get_value(objs[i], data.args[i]);
        }
        return s;
    });
    pb.set_result(sum);
}

void virt_1t(picobench::state& pb) { virt_benchmark(pb, 1); }
void virt_4t(picobench::state& pb) { virt_benchmark(pb, NUM_THREADS); }
void dynamix_1t(picobench::state& pb) { dynamix_benchmark(pb, 1); }
void dynamix_4t(picobench::state& pb) { dynamix_benchmark(pb, NUM_THREADS); }

PICOBENCH_SUITE("parallel message calls");
PICOBENCH(virt_1t).baseline();
PICOBENCH(virt_4t);
PICOBENCH(dynamix_1t);
PICOBENCH(dynamix_4t);

} // namespace calls

//////////////////////////////////////////////////////////////////////////////
// mutations
// each thread adds and removes a component to its own objects
// for C++ this measures how allocations scale
// for DynaMix the objects share a domain and the type lookups lock it (with a shared lock)

namespace mutations {

void cpp_benchmark(picobench::state& pb, unsigned num_threads) {
    const test_data data(pb);
    std::vector<entity> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_entity(v));

    auto count = run_parallel(pb, num_threads, objs.size(), [&](size_t begin, size_t end) {
        uintptr_t c = 0;
        for (size_t i = begin; i < end; ++i) {
            objs[i].add_extra();
            c += objs[i].components.size();
            objs[i].remove_extra();
        }
        return c;
    });
    pb.set_result(count);
}

void dynamix_benchmark(picobench::state& pb, unsigned num_threads) {
    const test_data data(pb);
    std::vector<dynamix::object> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_obj(v));
    // make sure the types exist
    add_extra(objs.front());
    remove_extra(objs.front());

    auto count = run_parallel(pb, num_threads, objs.size(), [&](size_t begin, size_t end) {
        uintptr_t c = 0;
        for (size_t i = begin; i < end; ++i) {
            add_extra(objs[i]);
            c += objs[i].num_mixins();
            remove_extra(objs[i]);
        }
        return c;
    });
    pb.set_result(count);
}

void cpp_1t(picobench::state& pb) { cpp_benchmark(pb, 1); }
void cpp_4t(picobench::state& pb) { cpp_benchmark(pb, NUM_THREADS); }
void dynamix_1t(picobench::state& pb) { dynamix_benchmark(pb, 1); }
void dynamix_4t(picobench::state& pb) { dynamix_benchmark(pb, NUM_THREADS); }

PICOBENCH_SUITE("parallel mutations");
PICOBENCH(cpp_1t).baseline();
PICOBENCH(cpp_4t);
PICOBENCH(dynamix_1t);
PICOBENCH(dynamix_4t);

} // namespace mutations

//////////////////////////////////////////////////////////////////////////////
// type creation
// threads create unique types in a shared domain, which requires an exclusive lock

namespace types {

void create_benchmark(picobench::state& pb, unsigned num_threads) {
    dbench::mixin_set ms(0);

    dbench::rng_t rng(seed);
    std::set<std::vector<uint32_t>> unique;
    std::vector<std::vector<const dynamix::mixin_info*>> queries;
    queries.reserve(size_t(pb.iterations()));
    while (queries.size() != size_t(pb.iterations())) {
        auto q = dbench::pick_unique(5, dbench::NUM_MIXINS, rng);
        if (unique.insert(q).second) {
            queries.push_back(ms.query(q, false));
        }
    }

    run_parallel(pb, num_threads, queries.size(), [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            ms.dom.get_type(queries[i]);
        }
        return uintptr_t(0);
    });
    pb.set_result(ms.dom.num_types());
}

void create_1t(picobench::state& pb) { create_benchmark(pb, 1); }
void create_4t(picobench::state& pb) { create_benchmark(pb, NUM_THREADS); }

PICOBENCH_SUITE("parallel type creation");
PICOBENCH(create_1t).baseline().iterations({1024, 16384});
PICOBENCH(create_4t).iterations({1024, 16384});

} // namespace types

int main(int argc, char* argv[]) {
    // with small dimensions the cost of waking up and joining the threads dominates
    return dbench::run(argc, argv, {262144, 1048576});
}
