// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <algorithm>
#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"

#include <cstdlib>

using namespace std;

#include "regression_tester.inl"

int main(int argc, char* argv[])
{
    picobench::runner r;

    // set some defaults in case there are no cmd-line arguments
#if defined(NDEBUG)
    r.set_default_samples(3);
    r.set_default_state_iterations({ 20000, 30000 });
#else
    r.set_default_samples(1);
    r.set_default_state_iterations({ 2000, 5000 });
#endif

    r.parse_cmd_line(argc, argv, "--pb");

    if(!r.should_run())
    {
        return r.error();
    }

    // prepare some global data
    int max_iters = 0;
    for (auto i : r.default_state_iterations())
    {
        max_iters = max(i, max_iters);
    }
    extern void fill_sample_data(size_t max_size);
    fill_sample_data(max_iters);

    auto report = r.run_benchmarks();

    report.to_text(std::cout);

    int i;
    for(i=1; i<argc; ++i)
    {
        if(strcmp(argv[i], "--test-perf-regression"))
        {
            break;
        }
    }

    if(i == argc) return 0; // no regression test required

    cout << "\n";

    bool b = true;

    try
    {
        // should be faster than the std::func baseline
        b &= test_regression(report, "setter", "msg_setter");
        b &= test_regression(report, "3x multi setter", "msg_setter");

        // all of these should be about the same speed
        b &= test_regression(report, "3x combine sum", "out_combinator", 1.1);
        b &= test_regression(report, "3x combine sum", "ret_combinator", 1.1);
    }
    catch (std::exception& ex)
    {
        cout << "Performance regression test error: " << ex.what() << "\n";
        return 1;
    }

    if (!b)
    {
        cerr << "Some performance regression tests failed!\n";
        return 1;
    }
    
    return 0;
}
