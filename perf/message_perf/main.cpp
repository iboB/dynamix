// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "perf.hpp"
using namespace std;

#include "timer.hpp"

// compare an unicast mixin message to
// regular functions
// virtual method
// std::function
//
// make sure link time optimizations are turned of
// gcc with no -flto
// msvc with no link time code generation

#define PERF(name, func) \
    srand(10); \
    t.start(name); \
    for(size_t i=0; i<A_LOT; ++i) \
    { \
        func; \
    } \
    t.avg(A_LOT)

#define PERF_SUM(name, func, getter) \
    PERF(name, func); \
    thesum = 0; \
    for(int i=0; i<OBJ_NUM; ++i) \
    { \
        thesum += getter; \
    } \
    cout << "sanity check: " << boolalpha << (thesum == A_LOT) << endl \


int main(int argc, char**)
{
    initialize_globals();
    size_t thesum = 0;

    timer t;

    #define access i%OBJ_NUM
    //#define access rand() % OBJ_NUM

    PERF_SUM("simple", regular_objects[access].add(argc), regular_objects[i].sum());
    PERF_SUM("virtual", ac_instances[access]->add(argc), ac_instances[i]->sum());
    PERF_SUM("std::func", f_add[access](argc), f_sum[i]());
    PERF_SUM("DynaMix", add(dm_objects[access], argc), sum(dm_objects[i]));

    PERF("simple noop", regular_objects[access].noop());
    PERF("virtual noop", ac_instances[access]->noop());
    PERF("std_func noop", f_noop[access]());
    PERF("DynaMix noop", noop(dm_objects[access]));

    return 0;
}
