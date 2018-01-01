// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "common.hpp"
#include "generated.hpp"

#define PICOBENCH_IMPLEMENT
#include "picobench.hpp"

#include "fast_allocator.hpp"

#include <iostream>

using namespace std;
using namespace dynamix;

PICOBENCH_SUITE("Type creation");

void new_type(picobench::state& s)
{
    s.start_timer();
    auto& templates = get_type_templates();
    s.stop_timer();
    assert(s.iterations() == templates.size());
}
PICOBENCH(new_type).samples(1).iterations({ 1023 });

PICOBENCH_SUITE("Object creation");

void create_mutate(picobench::state& s)
{
    auto& mutators = get_type_mutators();
    vector<object> objects(s.iterations());
    int i = 0;

    for (auto _ : s)
    {
        mutators[i % mutators.size()](objects[i]);
        ++i;
    }
}
PICOBENCH(create_mutate);

void type_template(picobench::state& s)
{
    auto& templates = get_type_templates();
    vector<object> objects(s.iterations());
    int i = 0;

    for (auto _ : s)
    {
        templates[i % templates.size()]->apply_to(objects[i]);
        ++i;
    }
}
PICOBENCH(type_template);

void type_template_alloc(picobench::state& s)
{
    fast_allocator alloc;
    auto& templates = get_type_templates();
    vector<object> objects;
    objects.reserve(s.iterations());
    for (int i = 0; i < s.iterations(); ++i)
    {
        objects.emplace_back(&alloc);
    }

    int i = 0;
    for (auto _ : s)
    {
        templates[i % templates.size()]->apply_to(objects[i]);
        ++i;
    }
}
PICOBENCH(type_template_alloc);

PICOBENCH_SUITE("Object mutation");

vector<object> create_objects(int n, object_allocator* a = nullptr)
{
    object_type_template t;
    t
        .add<mixin_3>()
        .add<mixin_4>()
        .add<mixin_6>()
        .add<mixin_7>()
        .add<mixin_9>()
        .add<mixin_10>()
        .create();

    vector<object> ret;
    ret.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        ret.emplace_back(t, a);
    }
    return ret;
}

void mutation(picobench::state& s)
{
    auto objects = create_objects(s.iterations());

    int i = 0;
    for (auto _ : s)
    {
        mutate(objects[i])
            .add<mixin_5>()
            .add<mixin_8>()
            .remove<mixin_9>();
        ++i;
    }
}
PICOBENCH(mutation);

void mutation_same_type_mutator(picobench::state& s)
{
    auto objects = create_objects(s.iterations());

    same_type_mutator mutator;
    mutator
        .add<mixin_5>()
        .add<mixin_8>()
        .remove<mixin_9>();

    int i = 0;
    for (auto _ : s)
    {
        mutator.apply_to(objects[i]);
        ++i;
    }
}
PICOBENCH(mutation_same_type_mutator).label("same_type_mutator");

void same_type_mutator_alloc(picobench::state& s)
{
    fast_allocator alloc;
    auto objects = create_objects(s.iterations(), &alloc);

    same_type_mutator mutator;
    mutator
        .add<mixin_5>()
        .add<mixin_8>()
        .remove<mixin_9>();

    int i = 0;
    for (auto _ : s)
    {
        mutator.apply_to(objects[i]);
        ++i;
    }
}
PICOBENCH(same_type_mutator_alloc);

int main(int argc, char* argv[])
{
    picobench::runner r;
    r.set_default_state_iterations({ 5000, 10000 });
    auto report = r.run_benchmarks();
    report.to_text(std::cout);
    return 0;
}