// picobench v1.04
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
//  1.04 (2018-02-06) * User data for benchmarks, which can be seen from states
//                    * `add_custom_duration` to states so the user can modify time
//                    * Text table format fixes
//                    * Custom cmd opts in runner
//                    * --version CLI command
//  1.03 (2018-01-05) Added helper methods for easier browsing of reports
//  1.02 (2018-01-04) Added parsing of command line
//  1.01 (2018-01-03) * Only taking the fastest sample into account
//                    * Set default number of samples to 2
//                    * Added CSV output
//  1.00 (2018-01-01) Initial release
//  0.01 (2017-12-28) Initial prototype release
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

#define PICOBENCH_VERSION 1.04
#define PICOBENCH_VERSION_STR "1.04"

#if defined(PICOBENCH_TEST_WITH_DOCTEST)
#   define PICOBENCH_TEST
#   define PICOBENCH_IMPLEMENT
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
    explicit state(int num_iterations, uintptr_t user_data = 0)
        : _iterations(num_iterations)
        , _user_data(user_data)
    {
        _PICOBENCH_ASSERT(_iterations > 0);
    }

    int iterations() const { return _iterations; }

    int64_t duration_ns() const { return _duration_ns; }
    void add_custom_duration(int64_t duration_ns) { _duration_ns += duration_ns; }

    uintptr_t user_data() const { return _user_data; }

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
    uintptr_t _user_data;
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
    benchmark& user_data(uintptr_t data) { _user_data = data; return *this; }

protected:
    friend class runner;

    benchmark(const char* name, benchmark_proc proc);

    const char* _name;
    const benchmark_proc _proc;
    bool _baseline = false;

    std::vector<int> _state_iterations;
    int _samples = 0;
    uintptr_t _user_data = 0;
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
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <cstring>
#include <cstdlib>

namespace picobench
{

class report
{
public:
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

        const benchmark* find_benchmark(const char* name) const
        {
            for (auto& b : benchmarks)
            {
                if (strcmp(b.name, name) == 0)
                    return &b;
            }

            return nullptr;
        }

        const benchmark* find_baseline() const
        {
            for (auto& b : benchmarks)
            {
                if (b.is_baseline)
                    return &b;
            }

            return nullptr;
        }
    };

    std::vector<suite> suites;

    const suite* find_suite(const char* name) const
    {
        for (auto& s : suites)
        {
            if (strcmp(s.name, name) == 0)
                return &s;
        }

        return nullptr;
    }

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
                        << setw(10) << fixed << setprecision(3) << double(bm.total_time_ns) / 1000000.0 << " |";

                    auto ns_op = (bm.total_time_ns / ps.first);
                    if (ns_op > 99999999)
                    {
                        int e = 0;
                        while (ns_op > 999999)
                        {
                            ++e;
                            ns_op /= 10;
                        }
                        out << ns_op << 'e' << e;
                    }
                    else
                    {
                        out << setw(8) << ns_op;
                    }

                    out << " |";

                    if (baseline == &bm)
                    {
                        out << "      - |";
                    }
                    else if (baseline)
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

class picostring
{
public:
    picostring() = default;
    explicit picostring(const char* text)
    {
        str = text;
        len = int(strlen(text));
    }

    const char* str;
    int len = 0;

    // checks whether other begins with this string
    bool cmp(const char* other) const
    {
        return strncmp(str, other, len) == 0;
    }
};

enum class report_output_format
{
    text,
    concise_text,
    csv
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
        _PICOBENCH_ASSERT(_error == 0 && _should_run);

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
                    b->_states.emplace(pos, iters, b->_user_data);
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

            for (auto& b : suite.benchmarks)
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

    const std::vector<int>& default_state_iterations() const
    {
        return _default_state_iterations;
    }

    void set_default_samples(int n)
    {
        _default_samples = n;
    }

    int default_samples() const
    {
        return _default_samples;
    }

    void add_cmd_opt(const char* cmd, const char* arg_desc, const char* cmd_desc, bool(*handler)(uintptr_t, const char*), uintptr_t user_data = 0)
    {
        cmd_line_option opt;
        opt.cmd = picostring(cmd);
        opt.arg_desc = picostring(arg_desc);
        opt.desc = cmd_desc;
        opt.handler = nullptr;
        opt.user_data = user_data;
        opt.user_handler = handler;
        _opts.push_back(opt);
    }

    // returns false if there were errors parsing the command line
    // all args starting with prefix are parsed
    // the others are ignored
    bool parse_cmd_line(int argc, const char* const argv[], const char* cmd_prefix = "-", std::ostream& out = std::cout, std::ostream& err = std::cerr)
    {
        _cmd_prefix = picostring(cmd_prefix);
        _stdout = &out;
        _stderr = &err;

        if (!_has_opts)
        {
            _opts.emplace_back("-iters=", "<n1,n2,n3,...>",
                "Sets default iterations for benchmarks",
                &runner::cmd_iters);
            _opts.emplace_back("-samples=", "<n>",
                "Sets default number of samples for benchmarks",
                &runner::cmd_samples);
            _opts.emplace_back("-out-fmt=", "<txt|con|csv>",
                "Outputs text or concise or csv",
                &runner::cmd_out_fmt);
            _opts.emplace_back("-output=", "<filename>",
                "Sets output filename or `stdout`",
                &runner::cmd_output);
            _opts.emplace_back("-no-run", "",
                "Doesn't run benchmarks",
                &runner::cmd_no_run);
            _opts.emplace_back("-version", "",
                "Show version info",
                &runner::cmd_version);
            _opts.emplace_back("-help", "",
                "Prints help",
                &runner::cmd_help);
            _has_opts = true;
        }

        for (int i = 1; i < argc; ++i)
        {
            if (!_cmd_prefix.cmp(argv[i]))
                continue;

            auto arg = argv[i] + _cmd_prefix.len;

            bool found = false;
            for (auto& opt : _opts)
            {
                if (opt.cmd.cmp(arg))
                {
                    found = true;
                    bool success = false;
                    if (opt.handler)
                    {
                        success = (this->*opt.handler)(arg + opt.cmd.len);
                    }
                    else
                    {
                        _PICOBENCH_ASSERT(opt.user_handler);
                        success = opt.user_handler(opt.user_data, arg + opt.cmd.len);
                    }

                    if (!success)
                    {
                        err << "Error: Bad command-line argument: " << argv[i] << "\n";
                        _error = 1;
                        return false;
                    }
                    break;
                }
            }

            if (!found)
            {
                err << "Error: Unknown command-line argument: " << argv[i] << "\n";
                _error = 1;
                return false;
            }
        }

        return true;
    }

    bool should_run() const { return _error == 0 && _should_run; }
    int error() const { return _error; }

    report_output_format preferred_output_format() const { return _output_format; }

    // can be nullptr (library-provided main will interpret it as stdout)
    const char* preferred_output_filename() const { return _output_file; }

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

            if (s.name && cur_suite_name && strcmp(s.name, cur_suite_name) == 0)
                return s.benchmarks;
        }
        ss.push_back({ cur_suite_name, {} });
        return ss.back().benchmarks;
    }

    // state
    int _error = 0;
    bool _should_run = true;
    report_output_format _output_format = report_output_format::text;
    const char* _output_file = nullptr; // nullptr means stdout

    // default data

    // default iterations per state per benchmark
    std::vector<int> _default_state_iterations;

    // default samples per benchmark
    int _default_samples;

    // command line parsing
    picostring _cmd_prefix;
    std::ostream* _stdout = nullptr;
    std::ostream* _stderr = nullptr;
    typedef bool (runner::*cmd_handler)(const char*); // internal handler
    typedef bool(*ext_handler)(uintptr_t user_data, const char* cmd_line); // external (user) handler
    struct cmd_line_option
    {
        cmd_line_option() = default;
        cmd_line_option(const char* c, const char* a, const char* d, cmd_handler h)
            : cmd(c)
            , arg_desc(a)
            , desc(d)
            , handler(h)
            , user_data(0)
            , user_handler(nullptr)
        {}
        picostring cmd;
        picostring arg_desc;
        const char* desc;
        cmd_handler handler; // may be nullptr for external handlers
        uintptr_t user_data; // passed as an argument to user handlers
        ext_handler user_handler;
    };
    bool _has_opts = false; // have opts been added to list
    std::vector<cmd_line_option> _opts;

    bool cmd_iters(const char* line)
    {
        std::vector<int> iters;
        auto p = line;
        while (true)
        {
            auto i = int(strtoul(p, nullptr, 10));
            if (i <= 0) return false;
            iters.push_back(i);
            p = strchr(p + 1, ',');
            if (!p) break;
            ++p;
        }
        if (iters.empty()) return false;
        _default_state_iterations = iters;
        return true;
    }

    bool cmd_samples(const char* line)
    {
        int samples = int(strtol(line, nullptr, 10));
        if (samples <= 0) return false;
        _default_samples = samples;
        return true;
    }

    bool cmd_no_run(const char* line)
    {
        if (*line) return false;
        _should_run = false;
        return true;
    }

    bool cmd_version(const char* line)
    {
        if (*line) return false;
        *_stdout << "picobench " PICOBENCH_VERSION_STR << "\n";
        _should_run = false;
        return true;
    }

    bool cmd_help(const char* line)
    {
        if (*line) return false;
        cmd_version(line);
        auto& cout = *_stdout;
        for (auto& opt : _opts)
        {
            cout << ' ' << _cmd_prefix.str << opt.cmd.str << opt.arg_desc.str;
            int w = 27 - (_cmd_prefix.len + opt.cmd.len + opt.arg_desc.len);
            for (int i = 0; i < w; ++i)
            {
                cout.put(' ');
            }
            cout << opt.desc << "\n";
        }
        _should_run = false;
        return true;
    }

    bool cmd_out_fmt(const char* line)
    {
        if (strcmp(line, "txt") == 0)
        {
            _output_format = report_output_format::text;
        }
        else if (strcmp(line, "con") == 0)
        {
            _output_format = report_output_format::concise_text;
        }
        else if (strcmp(line, "csv") == 0)
        {
            _output_format = report_output_format::csv;
        }
        else
        {
            return false;
        }
        return true;
    }

    bool cmd_output(const char* line)
    {
        if (strcmp(line, "stdout") != 0)
        {
            _output_file = line;
        }
        else
        {
            _output_file = nullptr;
        }
        return true;
    }
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
    r.parse_cmd_line(argc, argv);
    if (r.should_run())
    {
        auto report = r.run_benchmarks();
        std::ostream* out = &std::cout;
        std::ofstream fout;
        if (r.preferred_output_filename())
        {
            fout.open(r.preferred_output_filename());
            if (!fout.is_open())
            {
                std::cerr << "Error: Could not open output file `" << r.preferred_output_filename() << "`\n";
                return 1;
            }
            out = &fout;
        }

        switch (r.preferred_output_format())
        {
        case picobench::report_output_format::text:
            report.to_text(*out);
            break;
        case picobench::report_output_format::concise_text:
            report.to_text_concise(*out);
            break;
        case picobench::report_output_format::csv:
            report.to_csv(*out);
            break;
        }
    }
    return r.error();
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
    this_thread_sleep_for_ns((s.iterations() - 1) * 20);
    s.stop_timer();

    s.add_custom_duration(20);
}
PICOBENCH(a_c);

PICOBENCH_SUITE("test empty");

PICOBENCH_SUITE("test b");

static void b_a(picobench::state& s)
{
    CHECK(s.user_data() == 9088);
    for (auto _ : s)
    {
        this_thread_sleep_for_ns(75);
    }
}
PICOBENCH(b_a)
    .iterations({ 20, 30, 50 })
    .user_data(9088);

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

const picobench::report::suite& find_suite(const char* s, const picobench::report& r)
{
    auto suite = r.find_suite(s);
    REQUIRE(suite);
    return *suite;
}

#define cntof(ar) (sizeof(ar) / sizeof(ar[0]))

TEST_CASE("[picobench] test utils")
{
    const char* ar[] = { "test", "123", "asdf" };
    CHECK(cntof(ar) == 3);

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

TEST_CASE("[picobench] picostring")
{
    picostring str("test");
    CHECK(str.str == "test");
    CHECK(str.len == 4);
    CHECK(!str.cmp("tes"));
    CHECK(str.cmp("test"));
    CHECK(str.cmp("test123"));
}

const vector<int> default_iters = { 8, 64, 512, 4096, 8196 };
const int default_samples = 2;

TEST_CASE("[picobench] cmd line")
{
    {
        runner r;
        bool b = r.parse_cmd_line(0, {});
        CHECK(b);
        CHECK(r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_state_iterations() == default_iters);
        CHECK(r.default_samples() == default_samples);
        CHECK(!r.preferred_output_filename());
        CHECK(r.preferred_output_format() == report_output_format::text);
    }

    {
        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "-asdf" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-", sout, serr);
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Unknown command-line argument: -asdf\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == 1);
    }

    {
        runner r;
        const char* cmd_line[] = { "", "--no-run", "--iters=1,2,3", "--samples=54", "--out-fmt=con", "--output=stdout" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line);
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_samples() == 54);
        CHECK(r.default_state_iterations() == vector<int>({ 1, 2, 3 }));
        CHECK(!r.preferred_output_filename());
        CHECK(r.preferred_output_format() == report_output_format::concise_text);
    }

    {
        runner r;
        const char* cmd_line[] = { "", "--pb-no-run", "--pb-iters=1000,2000,3000", "-other-cmd1", "--pb-samples=54",
            "-other-cmd2", "--pb-out-fmt=csv", "--pb-output=foo.csv" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb");
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_samples() == 54);
        CHECK(r.default_state_iterations() == vector<int>({ 1000, 2000, 3000 }));
        CHECK(r.preferred_output_filename() == "foo.csv");
        CHECK(r.preferred_output_format() == report_output_format::csv);

    }

    {
        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--samples=xxx" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-", sout, serr);
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --samples=xxx\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == 1);
        CHECK(r.default_samples() == default_samples);
    }

    {
        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--iters=1,xxx,2" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-", sout, serr);
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --iters=1,xxx,2\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == 1);
        CHECK(r.default_state_iterations() == default_iters);
    }

    {
        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--out-fmt=asdf" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-", sout, serr);
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --out-fmt=asdf\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == 1);
        CHECK(r.preferred_output_format() == report_output_format::text);
    }

#define PB_VERSION_INFO "picobench " PICOBENCH_VERSION_STR "\n"

    {
        const char* v = PB_VERSION_INFO;

        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--pb-version" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb", sout, serr);
        CHECK(sout.str() == v);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
    }

#define PB_HELP \
        " --pb-iters=<n1,n2,n3,...>  Sets default iterations for benchmarks\n" \
        " --pb-samples=<n>           Sets default number of samples for benchmarks\n" \
        " --pb-out-fmt=<txt|con|csv> Outputs text or concise or csv\n" \
        " --pb-output=<filename>     Sets output filename or `stdout`\n" \
        " --pb-no-run                Doesn't run benchmarks\n" \
        " --pb-version               Show version info\n" \
        " --pb-help                  Prints help\n"

    {
        const char* help =
            PB_VERSION_INFO
            PB_HELP;

        runner r;
        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--pb-help" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb", sout, serr);
        CHECK(sout.str() == help);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
    }

    {
        const char* help =
            PB_VERSION_INFO
            " --pb-cmd-hi                Custom help\n"
            " --pb-cmd-bi=123            More custom help\n"
            PB_HELP;

        runner r;

        auto handler_hi = [](uintptr_t data, const char* cmd) -> bool {
            CHECK(data == 123);
            CHECK(*cmd == 0);
            return true;
        };

        r.add_cmd_opt("-cmd-hi", "", "Custom help", handler_hi, 123);

        auto handler_bi = [](uintptr_t data, const char* cmd) -> bool {
            CHECK(data == 98);
            CHECK(cmd == "123");
            return true;
        };

        r.add_cmd_opt("-cmd-bi=", "123", "More custom help", handler_bi, 98);

        ostringstream sout, serr;
        const char* cmd_line[] = { "", "--pb-help" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb", sout, serr);
        CHECK(sout.str() == help);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);

        sout.str(std::string());
        serr.str(std::string());

        const char* cmd_line2[] = { "",  "--zz-cmd-bi=123", "--zz-cmd-hi" };
        b = r.parse_cmd_line(cntof(cmd_line2), cmd_line2, "--zz", sout, serr);

        CHECK(sout.str().empty());
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(r.error() == 0);
    }
}

TEST_CASE("[picobench] test")
{
    runner r;
    CHECK(r.default_state_iterations() == default_iters);
    CHECK(r.default_samples() == default_samples);

    auto report = r.run_benchmarks();

    CHECK(report.suites.size() == 2);
    CHECK(!report.find_suite("asdf"));

    auto& a = find_suite("test a", report);
    CHECK(a.name == "test a");
    CHECK(a.benchmarks.size() == 3);
    CHECK(!a.find_benchmark("b_a"));

    auto& aa = a.benchmarks[0];
    CHECK(a.find_baseline() == &aa);
    CHECK(a.find_benchmark("a_a") == &aa);
    CHECK(aa.name == "a_a");
    CHECK(aa.is_baseline);
    CHECK(aa.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<aa.data.size(); ++i)
    {
        auto& d = aa.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 10);
    }

    auto& ab = a.benchmarks[1];
    CHECK(a.find_benchmark("a_b") == &ab);
    CHECK(ab.name == "a_b");
    CHECK(!ab.is_baseline);
    CHECK(ab.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<ab.data.size(); ++i)
    {
        auto& d = ab.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 11);
    }
    size_t j = 0;
    for (auto& elem : a_b_samples)
    {
        CHECK(elem.first == default_iters[j]);
        CHECK(elem.second == r.default_samples());
        ++j;
    }

    auto& ac = a.benchmarks[2];
    CHECK(a.find_benchmark("a_c") == &ac);
    CHECK(ac.name == "a_c");
    CHECK(!ac.is_baseline);
    CHECK(ac.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<ac.data.size(); ++i)
    {
        auto& d = ac.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 20);
    }

    auto& b = find_suite("test b", report);
    CHECK(b.name == "test b");
    CHECK(b.benchmarks.size() == 2);
    CHECK(!b.find_benchmark("b_b"));

    auto& ba = b.benchmarks[0];
    CHECK(ba.name == "b_a");
    CHECK(!ba.is_baseline);
    CHECK(ba.data.size() == 3);

    int state_iters_a[] = { 20, 30, 50 };
    for (size_t i = 0; i<ba.data.size(); ++i)
    {
        auto& d = ba.data[i];
        CHECK(d.dimension == state_iters_a[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 75);
    }

    auto& bb = b.benchmarks[1];
    CHECK(b.find_benchmark("something else") == &bb);
    CHECK(b.find_baseline() == &bb);
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