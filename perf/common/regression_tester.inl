
// test that perf of benchmark is better than perf of baseline_benchmark (or suite baseline if nullptr) times q
// returns false on fail
bool test_regression(const picobench::report& report,
    const char* suite_name, const char* benchmark, double q = 1, const char* baseline_benchmark = nullptr)
{
    auto suite = report.find_suite(suite_name);
    if (!suite) throw std::runtime_error("Can't find suite");

    auto bl = baseline_benchmark ? suite->find_benchmark(baseline_benchmark) : suite->find_baseline();
    if (!bl) throw std::runtime_error("Can't find baseline");

    auto bm = suite->find_benchmark(benchmark);
    if (!bm) throw std::runtime_error("Can't find benchmark");

    if (bl->data.size() != bm->data.size())
        throw std::runtime_error("Can't compare benchmarks");

    if (!baseline_benchmark) baseline_benchmark = "baseline";

    bool success = true;
    for (size_t i = 0; i < bl->data.size(); ++i)
    {
        auto& bld = bl->data[i];
        auto& bmd = bm->data[i];

        if (bld.dimension != bmd.dimension)
            throw std::runtime_error("Can't compare benchmark dimensions");

        if (double(bld.total_time_ns) * q <= double(bmd.total_time_ns))
        {
            cerr
                << benchmark << " is not faster than "
                << baseline_benchmark << " x " << q
                << " @ " << bld.dimension << "\n";
            success = false;
        }
    }

    return success;
}
