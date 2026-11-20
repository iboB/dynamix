#include "bmt-types.hpp"

#include <bench-rand.hpp>

#include <par/pchunk.hpp>

#include <picobench/picobench.hpp>
#include <bench-main.hpp>

// benchmark how operations scale with multiple threads
// the work (a fixed number of operations) is split between threads and the wall time is measured,
// so with perfect scaling the time per operation of the multi-threaded benchmarks would be
// 1/Num_Jobs of the single-threaded ones
// the results are only meaningful on machines with at least Num_Jobs cores.
constexpr uint32_t Num_Jobs = 4;
constexpr uint32_t Num_ParThreads = Num_Jobs - 1;
constexpr uint32_t Rand_Seed = 77;

struct test_data {
    std::vector<int> values;
    std::vector<int> args;
    explicit test_data(const picobench::state& pb) {
        dbench::rng_t rng(Rand_Seed);
        const auto n = size_t(pb.iterations());
        values.resize(n);
        for (auto& v : values) v = int(rng() % 10);
        args.resize(n);
        for (auto& a : args) a = int(rng() % 10);
    }
};

template <typename Func>
uintptr_t run_parallel(picobench::state& pb, uint32_t num_jobs, Func&& f) {
    uintptr_t results[Num_Jobs] = {};
    const par::run_opts opts = {
        .sched = par::schedule_static,
        .max_par = num_jobs
    };
    {
        picobench::scope bench(pb);
        par::pchunk(opts, size_t(pb.iterations()), [&](size_t begin, size_t end, const par::job_info& info) {
            assert(info.num_jobs <= Num_Jobs);
            results[info.job_index] += f(begin, end);
        });
    }
    uintptr_t sum = 0;
    for (auto r : results) sum += r;
    return sum;
}

///////////////////////////////////
// message calls:
// read-only objects are shared between threads
namespace calls {

void virt_benchmark(picobench::state& pb, unsigned num_jobs) {
    const test_data data(pb);
    std::vector<entity> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_entity(v));

    auto sum = run_parallel(pb, num_jobs, [&](size_t begin, size_t end) {
        uintptr_t s = 0;
        for (size_t i = begin; i < end; ++i) {
            s += objs[i].get_value(data.args[i]);
        }
        return s;
    });
    pb.set_result(sum);
}

void dynamix_benchmark(picobench::state& pb, unsigned num_jobs) {
    const test_data data(pb);
    std::vector<dynamix::object> objs;
    objs.reserve(data.values.size());
    for (auto v : data.values) objs.push_back(make_obj(v));

    auto sum = run_parallel(pb, num_jobs, [&](size_t begin, size_t end) {
        uintptr_t s = 0;
        for (size_t i = begin; i < end; ++i) {
            s += get_value(objs[i], data.args[i]);
        }
        return s;
    });
    pb.set_result(sum);
}

void virt_1t(picobench::state& pb) { virt_benchmark(pb, 1); }
void virt_4t(picobench::state& pb) { virt_benchmark(pb, Num_Jobs); }
void dynamix_1t(picobench::state& pb) { dynamix_benchmark(pb, 1); }
void dynamix_4t(picobench::state& pb) { dynamix_benchmark(pb, Num_Jobs); }

PICOBENCH_SUITE("parallel message calls");
PICOBENCH(virt_1t).baseline();
PICOBENCH(virt_4t);
PICOBENCH(dynamix_1t);
PICOBENCH(dynamix_4t);

}

int main(int argc, char* argv[]) {
    par::thread_pool::init_global(Num_ParThreads);

    return dbench::run_bench(argc, argv, {262144, 1048576});
}
