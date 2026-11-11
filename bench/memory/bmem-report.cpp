// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/domain.hpp>
#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/allocator.hpp>

#include <bench-resources.hpp>
#include <bench-mixin-set.hpp>
#include <bench-rand.hpp>

#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

// This is not a timing benchmark. It reports the memory used by DynaMix objects and types,
// and the number of allocations for common operations, by using a counting memory resource.
// The results are deterministic for a given platform and standard library.

using dbench::mixin_set;
using dbench::NUM_MIXINS;

// domain::get_type takes a span which doesn't bind to temporaries
const dynamix::type& get_type(dynamix::domain& dom, std::vector<const dynamix::mixin_info*> query) {
    return dom.get_type(query);
}

void report_objects() {
    std::printf("## Objects\n\n");
    std::printf("sizeof(dynamix::object): %zu\n\n", sizeof(dynamix::object));
    std::printf("Memory allocated for an object with N mixins, each with 8 bytes of data\n");
    std::printf("The overhead includes the pointer to the object which precedes each mixin\n\n");
    std::printf("| Mixins | External | Mixin data | Allocated | Overhead | Overhead/mixin | Allocations |\n");
    std::printf("|-------:|---------:|-----------:|----------:|---------:|---------------:|------------:|\n");

    mixin_set ms(0);
    auto& dom = ms.dom;
    dbench::counting_resource res;

    auto row = [&](uint32_t n, uint32_t ext) {
        auto query = ms.first_n(n, ext);
        auto& t = dom.get_type(query);
        res.reset_counters();
        auto before = res.bytes;
        dynamix::object obj(t, dynamix::allocator(&res));
        auto bytes = res.bytes - before;
        auto data = n * sizeof(uint64_t);
        auto overhead = bytes - data;
        std::printf("| %6u | %8u | %10zu | %9zu | %8zu | %14.1f | %11zu |\n",
            n, ext, data, bytes, overhead, n ? double(overhead) / n : 0.0, res.allocs);
    };

    for (uint32_t n : {1, 2, 5, 10}) row(n, 0);
    row(5, 1);
    row(5, 5);
    std::printf("\n");
}

void report_mutations() {
    std::printf("## Allocations per mutation\n\n");
    std::printf("An object with 5 internal mixins\n\n");
    std::printf("| Mutation | Allocations | Deallocations |\n");
    std::printf("|----------|------------:|--------------:|\n");

    mixin_set ms(0);
    auto& dom = ms.dom;
    dbench::counting_resource res;

    // func can put objects in keep, so that their destruction is not counted
    auto row = [&](const char* name, auto func) {
        std::vector<dynamix::object> keep;
        dynamix::object obj(get_type(dom, ms.first_n(5)), dynamix::allocator(&res));
        func(obj, keep); // create types
        dynamix::object obj2(get_type(dom, ms.first_n(5)), dynamix::allocator(&res));
        keep.clear();
        keep.reserve(1);
        res.reset_counters();
        func(obj2, keep);
        std::printf("| %s | %zu | %zu |\n", name, res.allocs, res.deallocs);
    };

    row("add an internal mixin", [&](dynamix::object& o, std::vector<dynamix::object>&) {
        dynamix::mutate(o, dynamix::add(ms.internal[5]->info));
    });
    row("add an external mixin", [&](dynamix::object& o, std::vector<dynamix::object>&) {
        dynamix::mutate(o, dynamix::add(ms.external[5]->info));
    });
    row("remove a mixin", [&](dynamix::object& o, std::vector<dynamix::object>&) {
        dynamix::mutate(o, dynamix::remove(ms.internal[0]->info));
    });
    row("reorder (to_back)", [&](dynamix::object& o, std::vector<dynamix::object>&) {
        dynamix::mutate(o, dynamix::to_back(ms.internal[0]->info));
    });
    row("copy", [&](dynamix::object& o, std::vector<dynamix::object>& keep) {
        keep.push_back(o.copy(dynamix::allocator(&res)));
    });
    row("move", [&](dynamix::object& o, std::vector<dynamix::object>& keep) {
        keep.push_back(std::move(o));
    });
    std::printf("\n");
}

void report_types() {
    std::printf("## Types\n\n");
    std::printf("Memory allocated by the domain for a new type with N mixins, each implementing F features\n");
    std::printf("Average for 100 types. Includes the entry in the type query cache.\n\n");
    std::printf("| Mixins | Features/mixin | Bytes/type |\n");
    std::printf("|-------:|---------------:|-----------:|\n");

    for (uint32_t f : {0, 1, 5}) {
        for (uint32_t n : {2, 5, 10}) {
            dbench::counting_resource res; // must outlive the domain
            mixin_set ms(f, dynamix::allocator(&res));
            auto& dom = ms.dom;

            dbench::rng_t rng(7);
            std::set<std::vector<uint32_t>> queries;
            while (queries.size() != 101) {
                queries.insert(dbench::pick_unique(n, NUM_MIXINS, rng));
            }

            auto i = queries.begin();
            get_type(dom, ms.query(*i, false)); // warm up the registries
            ++i;

            auto before = res.bytes;
            for (; i != queries.end(); ++i) {
                get_type(dom, ms.query(*i, false));
            }
            auto bytes = res.bytes - before;
            std::printf("| %6u | %14u | %10zu |\n", n, f, bytes / 100);
        }
    }
    std::printf("\n");
}

int main() {
    std::printf("# DynaMix memory report\n\n");
    report_objects();
    report_mutations();
    report_types();
    return 0;
}
