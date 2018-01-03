// picobench v1.01
// https://github.com/iboB/picobench
//
// A micro microbenchmarking library in a single header file
//
// MIT License
//
// Copyright(c) 2017-2018 Borislav Stanimirov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//                  VERSION HISTORY
//
//  0.01 (2017-12-28) Initial prototype release
//  1.00 (2018-01-01) Initial release
//  1.01 (2018-01-03) * Only taking the fastest sample into account
//                    * Set default number of samples to 2
//                    * Added CSV output
//
//
//                  EXAMPLE
//
// void my_function(); // the function you want to benchmark
//
// // write your benchmarking code in a function like this
// static void benchmark_my_function(picobench::state& state)
// {
//     // use the state in a range-based for loop to call your code
//     for (auto _ : state)
//         my_function();
// }
// // create a picobench with your benchmarking code
// PICOBENCH(benchmark_my_function);
//
//
//                  BASIC DOCUMENTATION
//
// A very brief usage guide follows. For more detailed documentation see the
// README here: https://github.com/iboB/picobench/blob/master/README.md
//
// Simply include this file wherever you need.
// You need to define PICOBENCH_IMPLEMENT_WITH_MAIN (or PICOBENCH_IMPLEMENT if
// you want to write your own main function) in one compilation unit to have 
// the implementation compiled there.
//
// The benchmark code must be a `void (picobench::state&)` function which 
// you have written. Benchmarks are registered using the `PICOBENCH` macro 
// where the only argument is the function's name.
//
// You can have multiple benchmarks in multiple files. All will be run when the
// executable starts.
//
// Typically a benchmark has a loop. To run the loop use the state argument in
// a range-based for loop in your function. The time spent looping is measured 
// for the benchmark. You can have initialization/deinitialization code outside
// of the loop and it won't be measured.
//
//
//                  TESTS
//
// Tests for the main features are included in this header file and use doctest 
// (https://github.com/onqtam/doctest). To run them, define 
// PICOBENCH_TEST_WITH_DOCTEST before including the header in a file which has
// doctest.h already included.
#pragma once

#include <cstdint>
#include <chrono>
#include <vector>

#if defined(PICOBENCH_TEST_WITH_DOCTEST)
#   define PICOBENCH_TEST
#   define PICOBENCH_IMPLEMENT
#   define private public
#endif

#if defined(PICOBENCH_DEBUG)
#   include <cassert>
#   define _PICOBENCH_ASSERT assert
#else
#   define _PICOBENCH_ASSERT(...)
#endif

#if defined(__GNUC__)
#   define PICOBENCH_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#   define PICOBENCH_INLINE  __forceinline
#else
#   define PICOBENCH_INLINE  inline
#endif

namespace picobench
{

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(PICOBENCH_TEST)
struct high_res_clock
{
    typedef long long rep;
    typedef std::nano period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<high_res_clock> time_point;
    static const bool is_steady = true;

    static time_point now();
};
#else
typedef std::chrono::high_resolution_clock high_res_clock;
#endif

class state
{
public:
    state(int num_iterations)
        : _iterations(num_iterations)
    {
        _PICOBENCH_ASSERT(_iterations > 0);
    }

    int iterations() const { return _iterations; }

    int64_t duration_ns() const { return _duration_ns; }

    void start_timer()
    {
        _start = high_res_clock::now();
    }

    void stop_timer()
    {
        auto duration = high_res_clock::now() - _start;
        _duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }

    struct iterator
    {
        PICOBENCH_INLINE
        iterator(state* parent)
            : _counter(parent->iterations())
            , _state(parent)
        {
            _PICOBENCH_ASSERT(_counter > 0);
        }

        PICOBENCH_INLINE
        iterator()
            : _counter(0)
            , _state(nullptr)
        {}

        PICOBENCH_INLINE
        iterator& operator++()
        {
            _PICOBENCH_ASSERT(_counter > 0);
            --_counter;
            return *this;
        }

        PICOBENCH_INLINE
        bool operator!=(const iterator&) const
        {
            if (_counter) return true;
            _state->stop_timer();
            return false;
        }

        PICOBENCH_INLINE
        int operator*() const
        {
            return 0;
        }

    private:
        int _counter;
        state* _state;
    };

    PICOBENCH_INLINE
    iterator begin()
    {
        start_timer();
        return iterator(this);
    }

    PICOBENCH_INLINE
    iterator end()
    {
        return iterator();
    }

private:
    high_res_clock::time_point _start;
    int64_t _duration_ns = 0;
    int _iterations;
};

// this can be used for manual measurement
class scope
{
public:
    scope(state& s)
        : _state(s)
    {
        _state.start_timer();
    }

    ~scope()
    {
        _state.stop_timer();
    }
private:
    state& _state;
};

typedef void(*benchmark_proc)(state&);

class benchmark
{
public:
    const char* name() const { return _name; }

    benchmark& iterations(std::vector<int> data) { _state_iterations = std::move(data); return *this; }
    benchmark& samples(int n) { _samples = n; return *this; }
    benchmark& label(const char* label) { _name = label; return *this; }
    benchmark& baseline(bool b = true) { _baseline = b; return *this; }

protected:
    friend class runner;

    benchmark(const char* name, benchmark_proc proc);

    const char* _name;
    const benchmark_proc _proc;
    bool _baseline = false;

    std::vector<int> _state_iterations;
    int _samples = 0;
};

class registry
{
public:
    static int set_test_suite(const char* name);
    static benchmark& new_benchmark(const char* name, benchmark_proc proc);
};

}

#define _PICOBENCH_PP_CAT(a, b) _PICOBENCH_PP_INTERNAL_CAT(a, b)
#define _PICOBENCH_PP_INTERNAL_CAT(a, b) a##b

#define PICOBENCH_SUITE(name) \
    static int _PICOBENCH_PP_CAT(picobench_suite, __LINE__) = \
    picobench::registry::set_test_suite(name)

#define PICOBENCH(func) \
    static auto& _PICOBENCH_PP_CAT(picobench, __LINE__) = \
    picobench::registry::new_benchmark(#func, func)

#if defined PICOBENCH_IMPLEMENT_WITH_MAIN
#   define PICOBENCH_IMPLEMENT
#   define PICOBENCH_IMPLEMENT_MAIN
#endif

#if defined PICOBENCH_IMPLEMENT

#include <random>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <map>
#include <memory>
#include <cstring>

namespace picobench
{

struct report
{
    struct benchmark_problem_space
    {
        int dimension; // number of iterations for the problem space
        int samples; // number of samples taken
        int64_t total_time_ns; // fastest sample!!!
    };
    struct benchmark
    {
        const char* name;
        bool is_baseline;
        std::vector<benchmark_problem_space> data;
    };

    struct suite
    {
        const char* name;
        std::vector<benchmark> benchmarks; // benchmark view
    };

    std::vector<suite> suites;

    void to_text(std::ostream& out) const
    {
        using namespace std;
        for (auto& suite : suites)
        {
            if (suite.name)
            {
                out << suite.name << ":\n";
            }

            line(out);
            out <<
                "   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second\n";
            line(out);

            auto problem_space_view = get_problem_space_view(suite);
            for (auto& ps : problem_space_view)
            {
                const problem_space_benchmark* baseline = nullptr;
                for (auto& bm : ps.second)
                {
                    if (bm.is_baseline)
                    {
                        baseline = &bm;
                        break;
                    }
                }

                for (auto& bm : ps.second)
                {
                    if (bm.is_baseline)
                    {
                        out << setw(23) << bm.name << " *";
                    }
                    else
                    {
                        out << setw(25) << bm.name;
                    }

                    out << " |"
                        << setw(8) << ps.first << " |"
                        << setw(10) << fixed << setprecision(3) << double(bm.total_time_ns) / 1000000.0 << " |"
                        << setw(8) << (bm.total_time_ns / ps.first) << " |";

                    if (baseline == &bm)
                    {
                        out << "      - |";
                    }
                    else if(baseline)
                    {
                        out << setw(7) << fixed << setprecision(3)
                            << double(bm.total_time_ns) / double(baseline->total_time_ns) << " |";
                    }
                    else
                    {
                        // no baseline to compare to
                        out << "    ??? |";
                    }

                    auto ops_per_sec = ps.first * (1000000000.0 / double(bm.total_time_ns));
                    out << setw(11) << fixed << setprecision(1) << ops_per_sec << "\n";
                }
            }
            line(out);
        }
    }

    void to_text_concise(std::ostream& out)
    {
        using namespace std;
        for (auto& suite : suites)
        {
            if (suite.name)
            {
                out << suite.name << ":\n";
            }

            line(out);

            out <<
                "   Name (baseline is *)   |  ns/op  | Baseline |  Ops/second\n";

            line(out);

            const benchmark* baseline = nullptr;
            for (auto& bm : suite.benchmarks)
            {
                if (bm.is_baseline)
                {
                    baseline = &bm;
                    break;
                }
            }
            _PICOBENCH_ASSERT(baseline);
            int64_t baseline_total_time = 0;
            int baseline_total_iterations = 0;
            for (auto& d : baseline->data)
            {
                baseline_total_time += d.total_time_ns;
                baseline_total_iterations += d.dimension;
            }
            int64_t baseline_ns_per_op = baseline_total_time / baseline_total_iterations;

            for (auto& bm : suite.benchmarks)
            {
                if (bm.is_baseline)
                {
                    out << setw(23) << bm.name << " *";
                }
                else
                {
                    out << setw(25) << bm.name;
                }

                int64_t total_time = 0;
                int total_iterations = 0;
                for (auto& d : bm.data)
                {
                    total_time += d.total_time_ns;
                    total_iterations += d.dimension;
                }
                int64_t ns_per_op = total_time / total_iterations;

                out << " |" << setw(8) << ns_per_op << " |";

                if (&bm == baseline)
                {
                    out << "        - |";
                }
                else
                {
                    out << setw(9) << fixed << setprecision(3)
                        << double(ns_per_op) / double(baseline_ns_per_op) << " |";
                }

                auto ops_per_sec = total_iterations * (1000000000.0 / double(total_time));
                out << setw(12) << fixed << setprecision(1) << ops_per_sec << "\n";
            }

            line(out);
        }
    }

    void to_csv(std::ostream& out, bool header = true) const
    {
        using namespace std;

        if (header)
        {
            out << "Suite,Benchmark,b,D,S,\"Total ns\",\"ns/op\",Baseline\n";
        }

        for (auto& suite : suites)
        {
            const benchmark* baseline = nullptr;
            for (auto& bm : suite.benchmarks)
            {
                if (bm.is_baseline)
                {
                    baseline = &bm;
                    break;
                }
            }
            _PICOBENCH_ASSERT(baseline);

            for (auto& bm : suite.benchmarks)
            {
                for (auto& d : bm.data)
                {
                    if (suite.name)
                    {
                        out << '"' << suite.name << '"';;
                    }
                    out << ",\"" << bm.name << "\",";
                    if (&bm == baseline)
                    {
                        out << '*';
                    }
                    out << ','
                        << d.dimension << ','
                        << d.samples << ','
                        << d.total_time_ns << ','
                        << (d.total_time_ns / d.dimension) << ',';
                    
                    if (baseline)
                    {
                        for (auto& bd : baseline->data)
                        {
                            if (bd.dimension == d.dimension)
                            {
                                out << fixed << setprecision(3) << (double(d.total_time_ns) / double(bd.total_time_ns));
                            }
                        }
                    }

                    out << '\n';
                }
            }
        }
    }

private:

    static void line(std::ostream& out)
    {
        for (int i = 0; i < 79; ++i) out.put('=');
        out.put('\n');
    }

    struct problem_space_benchmark
    {
        const char* name;
        bool is_baseline;
        int64_t total_time_ns; // fastest sample!!!
    };

    static std::map<int, std::vector<problem_space_benchmark>> get_problem_space_view(const suite& s)
    {
        std::map<int, std::vector<problem_space_benchmark>> res;
        for (auto& bm : s.benchmarks)
        {
            for (auto& d : bm.data)
            {
                auto& pvbs = res[d.dimension];
                pvbs.push_back({ bm.name, bm.is_baseline, d.total_time_ns });
            }
        }
        return res;
    }
};

class benchmark_impl : public benchmark
{
public:
    benchmark_impl(const char* name, benchmark_proc proc)
        : benchmark(name, proc)
    {}

private:
    friend class runner;

    // state
    std::vector<state> _states; // length is _samples * _state_iterations.size()
    std::vector<state>::iterator _istate;
};

class runner
{
public:
    runner()
        : _default_state_iterations({ 8, 64, 512, 4096, 8196 })
        , _default_samples(2)
    {}

    report run_benchmarks(int random_seed = -1)
    {
        if (random_seed == -1)
        {
            random_seed = std::random_device()();
        }

        std::minstd_rand rnd(random_seed);

        auto& registered_suites = suites();

        // vector of all benchmarks
        std::vector<benchmark_impl*> benchmarks;
        for (auto& suite : registered_suites)
        {
            // also identify a baseline in this loop
            // if there is no explicit one, set the first one as a baseline
            bool found_baseline = false;
            for (auto& rb : suite.benchmarks)
            {
                benchmarks.push_back(rb.get());
                if (rb->_baseline)
                {
                    found_baseline = true;
                }
            }

            if (!found_baseline && !suite.benchmarks.empty())
            {
                suite.benchmarks.front()->_baseline = true;
            }
        }

        // initialize benchmarks
        for (auto b : benchmarks)
        {
            std::vector<int>& state_iterations =
                b->_state_iterations.empty() ?
                _default_state_iterations :
                b->_state_iterations;

            if (b->_samples == 0)
                b->_samples = _default_samples;

            b->_states.reserve(state_iterations.size());

            // fill states while random shuffling them
            for (auto iters : state_iterations)
            {
                for (int i = 0; i < b->_samples; ++i)
                {
                    auto index = rnd() % (b->_states.size() + 1);
                    auto pos = b->_states.begin() + index;
                    b->_states.emplace(pos, iters);
                }
            }

            b->_istate = b->_states.begin();
        }

        // we run a random benchmark from it incrementing _istate for each
        // when _istate reaches _states.end(), we erase the benchmark
        // when the vector becomes empty, we're done
        while (!benchmarks.empty())
        {
            auto i = benchmarks.begin() + (rnd() % benchmarks.size());
            auto& b = *i;

            b->_proc(*b->_istate);

            ++b->_istate;

            if (b->_istate == b->_states.end())
            {
                benchmarks.erase(i);
            }
        }

        // generate report
        report rpt;

        rpt.suites.resize(registered_suites.size());
        auto rpt_suite = rpt.suites.begin();

        for (auto& suite : registered_suites)
        {
            rpt_suite->name = suite.name;

            // build benchmark view
            rpt_suite->benchmarks.resize(suite.benchmarks.size());
            auto rpt_benchmark = rpt_suite->benchmarks.begin();

            for(auto& b : suite.benchmarks)
            {
                rpt_benchmark->name = b->_name;
                rpt_benchmark->is_baseline = b->_baseline;

                std::vector<int>& state_iterations =
                    b->_state_iterations.empty() ?
                    _default_state_iterations :
                    b->_state_iterations;

                rpt_benchmark->data.reserve(state_iterations.size());
                for (auto d : state_iterations)
                {
                    rpt_benchmark->data.push_back({ d, 0, 0ll });
                }

                for (auto& state : b->_states)
                {
                    for (auto& d : rpt_benchmark->data)
                    {
                        if (state.iterations() == d.dimension)
                        {
                            if (d.total_time_ns == 0 || d.total_time_ns > state.duration_ns())
                            {
                                d.total_time_ns = state.duration_ns();
                            }
                            ++d.samples;
                        }
                    }
                }

#if defined(PICOBENCH_DEBUG)
                for (auto& d : rpt_benchmark->data)
                {
                    _PICOBENCH_ASSERT(d.samples == b->_samples);
                }
#endif

                ++rpt_benchmark;
            }

            ++rpt_suite;
        }

        return rpt;
    }

    void set_default_state_iterations(const std::vector<int>& data)
    {
        _default_state_iterations = data;
    }

    void set_default_samples(int n)
    {
        _default_samples = n;
    }

private:
    friend class registry;

    // global registration of all benchmarks
    using benchmarks_vector = std::vector<std::unique_ptr<benchmark_impl>>;
    struct rsuite
    {
        const char* name;
        benchmarks_vector benchmarks;
    };

    static std::vector<rsuite>& suites()
    {
        static std::vector<rsuite> b;
        return b;
    }

    static const char*& current_suite()
    {
        static const char* s = nullptr;
        return s;
    }

    static benchmarks_vector& benchmarks_for_current_suite()
    {
        const char* cur_suite_name = current_suite();
        auto& ss = suites();
        for (auto& s : ss)
        {
            if (s.name == cur_suite_name)
                return s.benchmarks;
            
            if(s.name && cur_suite_name && strcmp(s.name, cur_suite_name) == 0)
                return s.benchmarks;
        }
        ss.push_back({ cur_suite_name, {} });
        return ss.back().benchmarks;
    }

    // default data

    // default iterations per state per benchmark
    std::vector<int> _default_state_iterations;

    // default samples per benchmark
    int _default_samples;
};

benchmark::benchmark(const char* name, benchmark_proc proc)
    : _name(name)
    , _proc(proc)
{}

benchmark& registry::new_benchmark(const char* name, benchmark_proc proc)
{
    auto b = new benchmark_impl(name, proc);
    runner::benchmarks_for_current_suite().emplace_back(b);
    return *b;
}

int registry::set_test_suite(const char* name)
{
    runner::current_suite() = name;
    return 0;
}

#if (defined(_MSC_VER) || defined(__MINGW32__)) && !defined(PICOBENCH_TEST)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const long long high_res_clock_freq = []() -> long long
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}();

high_res_clock::time_point high_res_clock::now()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return time_point(duration((t.QuadPart * rep(period::den)) / high_res_clock_freq));
}
#endif
}

#endif

#if defined PICOBENCH_IMPLEMENT_MAIN
int main(int argc, char* argv[])
{
    picobench::runner r;
    auto report = r.run_benchmarks();
    report.to_text(std::cout);
    //report.to_text_concise(std::cout);
    //report.to_csv(std::cout);
    return 0;
}
#endif

#if defined(PICOBENCH_TEST)

// fake time keeping functions for the tests
namespace picobench
{

void this_thread_sleep_for_ns(uint64_t ns);

template <class Rep, class Period>
void this_thread_sleep_for(const std::chrono::duration<Rep, Period>& duration)
{
    this_thread_sleep_for_ns(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
}

#if defined PICOBENCH_IMPLEMENT
static struct fake_time
{
    uint64_t now;
} the_time;

void this_thread_sleep_for_ns(uint64_t ns)
{
    the_time.now += ns;
}

high_res_clock::time_point high_res_clock::now()
{
    auto ret = time_point(duration(the_time.now));
    return ret;
}
#endif

}

#endif

#if defined(PICOBENCH_TEST_WITH_DOCTEST)

#include <string>
#include <sstream>

using namespace picobench;
using namespace std;

PICOBENCH_SUITE("test a");

static void a_a(picobench::state& s)
{
    for (auto _ : s)
    {
        this_thread_sleep_for_ns(10);
    }
}
PICOBENCH(a_a);

map<int, int> a_b_samples;
static void a_b(picobench::state& s)
{
    uint64_t time = 11;
    if (a_b_samples.find(s.iterations()) == a_b_samples.end())
    {
        // slower first time
        time = 32;
    }

    ++a_b_samples[s.iterations()];
    for (auto _ : s)
    {
        this_thread_sleep_for_ns(time);
    }
}
PICOBENCH(a_b);

static void a_c(picobench::state& s)
{
    s.start_timer();
    this_thread_sleep_for_ns(s.iterations() * 20);
    s.stop_timer();
}
PICOBENCH(a_c);

PICOBENCH_SUITE("test empty");

PICOBENCH_SUITE("test b");

static void b_a(picobench::state& s)
{
    for (auto _ : s)
    {
        this_thread_sleep_for_ns(75);
    }
}
PICOBENCH(b_a)
    .iterations({ 20, 30, 50 });

map<int, int> b_b_samples;

static void b_b(picobench::state& s)
{
    uint64_t time = 111;
    if (b_b_samples.find(s.iterations()) == b_b_samples.end())
    {
        // faster first time
        time = 100;
    }

    ++b_b_samples[s.iterations()];
    picobench::scope scope(s);
    this_thread_sleep_for_ns(s.iterations() * time);
}
PICOBENCH(b_b)
    .baseline()
    .label("something else")
    .samples(15)
    .iterations({ 10, 20, 30 });

const report::suite& find_suite(const string& s, const report& r)
{
    for (auto& suite : r.suites)
    {
        if (s == suite.name)
            return suite;
    }

    FAIL("missing suite" << s);
    return r.suites.front(); // to avoid noreturn warning
}

TEST_CASE("[picobench] clock test")
{
    auto start = high_res_clock::now();
    this_thread_sleep_for_ns(1234);
    auto end = high_res_clock::now();

    auto duration = end - start;
    CHECK(duration == std::chrono::nanoseconds(1234));

    start = high_res_clock::now();
    this_thread_sleep_for(std::chrono::milliseconds(987));
    end = high_res_clock::now();
    duration = end - start;
    CHECK(duration == std::chrono::milliseconds(987));
}

TEST_CASE("[picobench] test")
{
    runner r;
    const vector<int> iters = { 8, 64, 512, 4096, 8196 };
    CHECK(r._default_state_iterations == iters);
    CHECK(r._default_samples == 2);

    auto report = r.run_benchmarks();

    CHECK(report.suites.size() == 2);

    auto& a = find_suite("test a", report);
    CHECK(a.name == "test a");
    CHECK(a.benchmarks.size() == 3);

    auto& aa = a.benchmarks[0];
    CHECK(aa.name == "a_a");
    CHECK(aa.is_baseline);
    CHECK(aa.data.size() == r._default_state_iterations.size());

    for (size_t i = 0; i<aa.data.size(); ++i)
    {
        auto& d = aa.data[i];
        CHECK(d.dimension == r._default_state_iterations[i]);
        CHECK(d.samples == r._default_samples);
        CHECK(d.total_time_ns == d.dimension * 10);
    }

    auto& ab = a.benchmarks[1];
    CHECK(ab.name == "a_b");
    CHECK(!ab.is_baseline);
    CHECK(ab.data.size() == r._default_state_iterations.size());

    for (size_t i=0; i<ab.data.size(); ++i)
    {
        auto& d = ab.data[i];
        CHECK(d.dimension == r._default_state_iterations[i]);
        CHECK(d.samples == r._default_samples);
        CHECK(d.total_time_ns == d.dimension * 11);
    }
    size_t j = 0;
    for (auto& elem : a_b_samples)
    {
        CHECK(elem.first == iters[j]);
        CHECK(elem.second == r._default_samples);
        ++j;
    }

    auto& ac = a.benchmarks[2];
    CHECK(ac.name == "a_c");
    CHECK(!ac.is_baseline);
    CHECK(ac.data.size() == r._default_state_iterations.size());

    for (size_t i = 0; i<ac.data.size(); ++i)
    {
        auto& d = ac.data[i];
        CHECK(d.dimension == r._default_state_iterations[i]);
        CHECK(d.samples == r._default_samples);
        CHECK(d.total_time_ns == d.dimension * 20);
    }

    auto& b = find_suite("test b", report);
    CHECK(b.name == "test b");
    CHECK(b.benchmarks.size() == 2);

    auto& ba = b.benchmarks[0];
    CHECK(ba.name == "b_a");
    CHECK(!ba.is_baseline);
    CHECK(ba.data.size() == 3);

    int state_iters_a[] = { 20, 30, 50 };
    for (size_t i = 0; i<ba.data.size(); ++i)
    {
        auto& d = ba.data[i];
        CHECK(d.dimension == state_iters_a[i]);
        CHECK(d.samples == r._default_samples);
        CHECK(d.total_time_ns == d.dimension * 75);
    }

    auto& bb = b.benchmarks[1];
    CHECK(bb.name == "something else");
    CHECK(bb.is_baseline);
    CHECK(bb.data.size() == 3);

    int state_iters_b[] = { 10, 20, 30 };
    for (size_t i = 0; i<bb.data.size(); ++i)
    {
        auto& d = bb.data[i];
        CHECK(d.dimension == state_iters_b[i]);
        CHECK(d.samples == 15);
        CHECK(d.total_time_ns == d.dimension * 100);
    }

    j = 0;
    for (auto& elem : b_b_samples)
    {
        CHECK(elem.first == state_iters_b[j]);
        CHECK(elem.second == 15);
        ++j;
    }

    ostringstream sout;
    report.to_text_concise(sout);
    const char* concise =
        "test a:\n"
        "===============================================================================\n"
        "   Name (baseline is *)   |  ns/op  | Baseline |  Ops/second\n"
        "===============================================================================\n"
        "                    a_a * |      10 |        - | 100000000.0\n"
        "                      a_b |      11 |    1.100 |  90909090.9\n"
        "                      a_c |      20 |    2.000 |  50000000.0\n"
        "===============================================================================\n"
        "test b:\n"
        "===============================================================================\n"
        "   Name (baseline is *)   |  ns/op  | Baseline |  Ops/second\n"
        "===============================================================================\n"
        "                      b_a |      75 |    0.750 |  13333333.3\n"
        "         something else * |     100 |        - |  10000000.0\n"
        "===============================================================================\n";

    CHECK(sout.str() == concise);

    const char* txt = 
        "test a:\n"
        "===============================================================================\n"
        "   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second\n"
        "===============================================================================\n"
        "                    a_a * |       8 |     0.000 |      10 |      - |100000000.0\n"
        "                      a_b |       8 |     0.000 |      11 |  1.100 | 90909090.9\n"
        "                      a_c |       8 |     0.000 |      20 |  2.000 | 50000000.0\n"
        "                    a_a * |      64 |     0.001 |      10 |      - |100000000.0\n"
        "                      a_b |      64 |     0.001 |      11 |  1.100 | 90909090.9\n"
        "                      a_c |      64 |     0.001 |      20 |  2.000 | 50000000.0\n"
        "                    a_a * |     512 |     0.005 |      10 |      - |100000000.0\n"
        "                      a_b |     512 |     0.006 |      11 |  1.100 | 90909090.9\n"
        "                      a_c |     512 |     0.010 |      20 |  2.000 | 50000000.0\n"
        "                    a_a * |    4096 |     0.041 |      10 |      - |100000000.0\n"
        "                      a_b |    4096 |     0.045 |      11 |  1.100 | 90909090.9\n"
        "                      a_c |    4096 |     0.082 |      20 |  2.000 | 50000000.0\n"
        "                    a_a * |    8196 |     0.082 |      10 |      - |100000000.0\n"
        "                      a_b |    8196 |     0.090 |      11 |  1.100 | 90909090.9\n"
        "                      a_c |    8196 |     0.164 |      20 |  2.000 | 50000000.0\n"
        "===============================================================================\n"
        "test b:\n"
        "===============================================================================\n"
        "   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second\n"
        "===============================================================================\n"
        "         something else * |      10 |     0.001 |     100 |      - | 10000000.0\n"
        "                      b_a |      20 |     0.002 |      75 |  0.750 | 13333333.3\n"
        "         something else * |      20 |     0.002 |     100 |      - | 10000000.0\n"
        "                      b_a |      30 |     0.002 |      75 |  0.750 | 13333333.3\n"
        "         something else * |      30 |     0.003 |     100 |      - | 10000000.0\n"
        "                      b_a |      50 |     0.004 |      75 |    ??? | 13333333.3\n"
        "===============================================================================\n";

    sout.str(string());
    report.to_text(sout);
    CHECK(sout.str() == txt);

    const char* csv =
        "Suite,Benchmark,b,D,S,\"Total ns\",\"ns/op\",Baseline\n"
        "\"test a\",\"a_a\",*,8,2,80,10,1.000\n"
        "\"test a\",\"a_a\",*,64,2,640,10,1.000\n"
        "\"test a\",\"a_a\",*,512,2,5120,10,1.000\n"
        "\"test a\",\"a_a\",*,4096,2,40960,10,1.000\n"
        "\"test a\",\"a_a\",*,8196,2,81960,10,1.000\n"
        "\"test a\",\"a_b\",,8,2,88,11,1.100\n"
        "\"test a\",\"a_b\",,64,2,704,11,1.100\n"
        "\"test a\",\"a_b\",,512,2,5632,11,1.100\n"
        "\"test a\",\"a_b\",,4096,2,45056,11,1.100\n"
        "\"test a\",\"a_b\",,8196,2,90156,11,1.100\n"
        "\"test a\",\"a_c\",,8,2,160,20,2.000\n"
        "\"test a\",\"a_c\",,64,2,1280,20,2.000\n"
        "\"test a\",\"a_c\",,512,2,10240,20,2.000\n"
        "\"test a\",\"a_c\",,4096,2,81920,20,2.000\n"
        "\"test a\",\"a_c\",,8196,2,163920,20,2.000\n"
        "\"test b\",\"b_a\",,20,2,1500,75,0.750\n"
        "\"test b\",\"b_a\",,30,2,2250,75,0.750\n"
        "\"test b\",\"b_a\",,50,2,3750,75,\n"
        "\"test b\",\"something else\",*,10,15,1000,100,1.000\n"
        "\"test b\",\"something else\",*,20,15,2000,100,1.000\n"
        "\"test b\",\"something else\",*,30,15,3000,100,1.000\n";

    sout.str(string());
    report.to_csv(sout);
    CHECK(sout.str() == csv);
}
#endif