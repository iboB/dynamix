// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmc-common.hpp"

#include <dynamix/type.hpp>

#include <bench-rand.hpp>

#define PICOBENCH_IMPLEMENT
#include <bench-main.hpp>

// This benchmark compares different ways of calling functionality of objects.
// Unlike msg-unicast and msg-multicast, which compare the cost of dispatch with different memory access patterns,
// here objects are always accessed sequentially and the focus is on the cost of the call itself.
// The DynaMix objects have 10 mixins and the called one is in the middle.
// Since feature lookup is constant-time, its position doesn't matter.
// The C++ objects have the same amount of data (10 ints).
// With the small dimension all data fits in the cache and the cost of the call itself is measured.
// With the big one the memory footprint of the objects also matters.

constexpr uint32_t seed = 101;

struct test_data {
    std::vector<int> values; // values[i] is the value of the i-th object
    std::vector<int> args; // args[i] is the argument for the i-th call
    std::vector<bool> tags; // tags[i] is true if the i-th object is tagged (for queries)

    explicit test_data(const picobench::state& pb) {
        dbench::rng_t rng(seed);
        const auto n = size_t(pb.iterations());
        values.resize(n);
        for (auto& v : values) v = int(rng() % 10);
        args.resize(n);
        for (auto& a : args) a = int(rng() % 10);
        tags.resize(n);
        for (size_t i = 0; i < n; ++i) tags[i] = rng() % 2 == 0;
    }
};

std::vector<dynamix::object> make_objects(const test_data& data, dynamix::object (*factory)(int)) {
    std::vector<dynamix::object> ret;
    ret.reserve(data.values.size());
    for (auto v : data.values) {
        ret.push_back(factory(v));
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// call kinds
// all return the same sum

void direct(picobench::state& pb) {
    const test_data data(pb);
    std::vector<std::unique_ptr<plain_value>> objs;
    for (auto v : data.values) objs.push_back(make_plain_value(v));

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += objs[i]->get_value(data.args[i]);
        }
    }
    pb.set_result(sum);
}

void virt(picobench::state& pb) {
    const test_data data(pb);
    std::vector<std::unique_ptr<virt_base>> objs;
    for (auto v : data.values) objs.push_back(make_virt_value(v));

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += objs[i]->get_value(data.args[i]);
        }
    }
    pb.set_result(sum);
}

void msg(picobench::state& pb) {
    const test_data data(pb);
    auto objs = make_objects(data, make_obj);

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += get_value(objs[i], data.args[i]);
        }
    }
    pb.set_result(sum);
}

// the entity-component style: get a specific mixin and call its method
// this is not polymorphic: the calling code must know which mixin provides the functionality
void get_mixin(picobench::state& pb) {
    const test_data data(pb);
    auto objs = make_objects(data, make_obj);

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += objs[i].get<value_holder>()->get_value(data.args[i]);
        }
    }
    pb.set_result(sum);
}

// a message whose top implementation calls the next one: two calls per message call
void msg_next_impl(picobench::state& pb) {
    const test_data data(pb);
    auto objs = make_objects(data, make_obj_with_decorator);

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += get_value(objs[i], data.args[i]);
        }
    }
    pb.set_result(sum);
}

PICOBENCH_SUITE("call kinds");
PICOBENCH(direct).baseline();
PICOBENCH(virt);
PICOBENCH(msg);
PICOBENCH(get_mixin);
PICOBENCH(msg_next_impl);

//////////////////////////////////////////////////////////////////////////////
// default implementation

void implemented(picobench::state& pb) {
    const test_data data(pb);
    auto objs = make_objects(data, make_obj);

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += ident(objs[i], data.args[i]);
        }
    }
    pb.set_result(sum);
}

void default_impl(picobench::state& pb) {
    const test_data data(pb);
    auto objs = make_objects(data, make_obj);

    uintptr_t sum = 0;
    {
        picobench::scope benchmark(pb);
        for (size_t i = 0; i < objs.size(); ++i) {
            sum += ident_default(objs[i], data.args[i]);
        }
    }
    pb.set_result(sum);
}

PICOBENCH_SUITE("default implementation");
PICOBENCH(implemented).baseline();
PICOBENCH(default_impl);

//////////////////////////////////////////////////////////////////////////////
// queries
// half of the objects have the mixin tagged (at the back of the type)
// all queries count the tagged objects

std::vector<dynamix::object> make_query_objects(const test_data& data) {
    std::vector<dynamix::object> ret;
    ret.reserve(data.values.size());
    for (size_t i = 0; i < data.values.size(); ++i) {
        ret.push_back(data.tags[i] ? make_tagged_obj(data.values[i]) : make_obj(data.values[i]));
    }
    return ret;
}

template <typename Query>
void query_benchmark(picobench::state& pb, Query q) {
    const test_data data(pb);
    auto objs = make_query_objects(data);

    uintptr_t count = 0;
    {
        picobench::scope benchmark(pb);
        for (auto& o : objs) {
            count += q(o);
        }
    }
    pb.set_result(count);
}

void has_by_type(picobench::state& pb) {
    query_benchmark(pb, [](const dynamix::object& o) { return o.has<tagged>(); });
}

void get_by_type(picobench::state& pb) {
    query_benchmark(pb, [](const dynamix::object& o) { return o.get<tagged>() != nullptr; });
}

// linear: compares the name to the names of all mixins in the type
void has_by_name(picobench::state& pb) {
    query_benchmark(pb, [](const dynamix::object& o) { return o.has("tagged"); });
}

void implements(picobench::state& pb) {
    query_benchmark(pb, [](const dynamix::object& o) { return o.get_type().implements_strong<tag_msg>(); });
}

// not cached: calls the type class match function every time
void type_class(picobench::state& pb) {
    query_benchmark(pb, [](const dynamix::object& o) { return o.is_of<is_tagged>(); });
}

PICOBENCH_SUITE("queries");
PICOBENCH(has_by_type).baseline();
PICOBENCH(get_by_type);
PICOBENCH(has_by_name);
PICOBENCH(implements);
PICOBENCH(type_class);

int main(int argc, char* argv[]) {
    return dbench::run(argc, argv, {1024, 65536});
}
