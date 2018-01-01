// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

// compare an unicast mixin message to
// virtual method and std::function
//
// make sure link time optimizations are turned of
// gcc with no -flto
// msvc with no link time code generation
#include "perf.hpp"

#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"

#include <cstdlib>

using namespace std;

PICOBENCH_SUITE("noop");

static void virtual_noop(picobench::state& s)
{
    vector<abstract_class*> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_abstract_class(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt++]->noop();
    }

    for (auto ptr : data)
    {
        delete ptr;
    }
}
PICOBENCH(virtual_noop);

static void std_func_noop(picobench::state& s)
{
    vector<std_func_object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_std_func(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt++].noop();
    }

    for (auto elem : data)
    {
        elem.release();
    }
}
PICOBENCH(std_func_noop).baseline();


static void msg_noop(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_object(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        noop(data[cnt++]);
    }
}
PICOBENCH(msg_noop);

PICOBENCH_SUITE("setter");

static void virtual_setter(picobench::state& s)
{
    vector<abstract_class*> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_abstract_class(rand()));
    }

    vector<int> ints;
    ints.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        ints.push_back(rand());
    }

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt]->add(ints[cnt]);
        ++cnt;
    }

    int sum_i = 0, sum_v = 0;
    for (int i = 0; i < s.iterations(); ++i)
    {
        sum_i += ints[i];
        sum_v += data[i]->sum();
        delete data[i];
    }

    assert(sum_i == sum_v);
}
PICOBENCH(virtual_setter);

static void std_func_setter(picobench::state& s)
{
    vector<std_func_object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_std_func(rand()));
    }

    vector<int> ints;
    ints.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        ints.push_back(rand());
    }

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt].add(ints[cnt]);
        ++cnt;
    }

    int sum_i = 0, sum_v = 0;
    for (int i = 0; i < s.iterations(); ++i)
    {
        sum_i += ints[i];
        sum_v += data[i].sum();
        data[i].release();
    }

    assert(sum_i == sum_v);
}
PICOBENCH(std_func_setter).baseline();

static void msg_setter(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_object(rand()));
    }

    vector<int> ints;
    ints.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        ints.push_back(rand());
    }

    int cnt = 0;
    for (auto _ : s)
    {
        add(data[cnt], ints[cnt]);
        ++cnt;
    }

    int sum_i = 0, sum_v = 0;
    for (int i = 0; i < s.iterations(); ++i)
    {
        sum_i += ints[i];
        sum_v += sum(data[i]);
    }

    assert(sum_i == sum_v);
}
PICOBENCH(msg_setter);

int main(int argc, char* argv[])
{
    picobench::runner r;
    r.set_default_state_iterations({ 20000, 50000 });
    auto report = r.run_benchmarks();
    report.to_text(std::cout);
    return 0;
}
