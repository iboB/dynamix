// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

// compare an multicast mixin messages to
// virtual method and std::function
//
// make sure link time optimizations are turned of
// gcc with no -flto
// msvc with no link time code generation
#include "perf.hpp"
#include "picobench.hpp"

using namespace std;

PICOBENCH_SUITE("3x multi noop");

static void virtual_noop(picobench::state& s)
{
    vector<abstract_multi> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_abstract_multi(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        data[cnt++].noop();
    }
}
PICOBENCH(virtual_noop);

static void std_func_noop(picobench::state& s)
{
    vector<std_func_multi> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_std_multi(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        for (auto& func : data[cnt].noops)
            func();

        ++cnt;
    }

    for (auto elem : data)
    {
        for (auto& func : elem.release)
            func();
    }
}
PICOBENCH(std_func_noop).baseline();

static void msg_noop(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_multi_object(rand()));
    }

    int cnt = 0;
    for (auto _ : s)
    {
        multi_noop(data[cnt++]);
    }
}
PICOBENCH(msg_noop);

PICOBENCH_SUITE("3x multi setter");

static void virtual_setter(picobench::state& s)
{
    vector<abstract_multi> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_abstract_multi(rand()));
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
    }

    assert(sum == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(virtual_setter);

static void std_func_setter(picobench::state& s)
{
    vector<std_func_multi> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.push_back(new_std_multi(rand()));
    }

    auto& ints = random_ints();

    int cnt = 0;
    for (auto _ : s)
    {
        int i = ints[cnt];
        for(auto& f : data[cnt].adds)
            f(i);        
        ++cnt;
    }

    unsigned sum = 0;
    for (auto d : data)
    {
        for (auto& f : d.sums)
            sum += f();
        for (auto& f : d.release)
            f();
    }

    assert(sum == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(std_func_setter).baseline();

static void msg_setter(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_multi_object(rand()));
    }

    auto& ints = random_ints();
    int cnt = 0;
    for (auto _ : s)
    {
        multi_add(data[cnt], ints[cnt]);
        ++cnt;
    }

    unsigned sum = 0;
    for(auto& o : data)
    {
        multi_sum_out(o, sum);
    }

    assert(sum == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(msg_setter);

PICOBENCH_SUITE("3x combine sum");

static void msg_sum_out(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_multi_object(rand()));
    }

    auto& ints = random_ints();
    
    for (int i=0; i<s.iterations(); ++i)
    {
        multi_add(data[i], ints[i]);
    }

    int cnt = 0;
    unsigned sum = 0;
    for (auto _ : s)
    {
        multi_sum_out(data[cnt], sum);
        ++cnt;
    }

    assert(sum == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(msg_sum_out);

static void out_combinator(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_multi_object(rand()));
    }

    auto& ints = random_ints();

    for (int i = 0; i<s.iterations(); ++i)
    {
        multi_add(data[i], ints[i]);
    }

    int cnt = 0;
    dynamix::combinators::sum<unsigned> sc;
    for (auto _ : s)
    {
        multi_sum(data[cnt], sc);
        ++cnt;
    }

    assert(sc.result() == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(out_combinator);


static void ret_combinator(picobench::state& s)
{
    vector<dynamix::object> data;
    data.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        data.emplace_back(new_multi_object(rand()));
    }

    auto& ints = random_ints();

    for (int i = 0; i<s.iterations(); ++i)
    {
        multi_add(data[i], ints[i]);
    }

    int cnt = 0;
    unsigned sum = 0;
    for (auto _ : s)
    {
        sum += multi_sum<dynamix::combinators::sum>(data[cnt]);
        ++cnt;
    }

    assert(sum == 3 * random_ints_partial_sums()[s.iterations() - 1]);
}
PICOBENCH(ret_combinator);