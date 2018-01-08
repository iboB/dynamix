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
#include "picobench.hpp"

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

    auto& ints = random_ints();

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt]->add(ints[cnt]);
        ++cnt;
    }

    unsigned sum = 0;
    for (auto& d : data)
    {
        sum += d->sum();
        delete d;
    }

    assert(sum == random_ints_partial_sums()[s.iterations() - 1]);
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

    auto& ints = random_ints();

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt].add(ints[cnt]);
        ++cnt;
    }

    unsigned sum = 0;
    for (auto& d : data)
    {
        sum += d.sum();
        d.release();
    }

    assert(sum == random_ints_partial_sums()[s.iterations() - 1]);
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
    
    auto& ints = random_ints();

    int cnt = 0;
    for (auto _ : s)
    {
        add(data[cnt], ints[cnt]);
        ++cnt;
    }

    unsigned isum = 0;
    for (auto& d : data)
    {
        isum += sum(d);
    }

    assert(isum == random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(msg_setter);
