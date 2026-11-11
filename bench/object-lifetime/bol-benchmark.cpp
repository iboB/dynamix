// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/type_mutation.hpp>
#include <dynamix/type.hpp>
#include <dynamix/allocator.hpp>

#include <bench-resources.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#define PICOBENCH_IMPLEMENT
#include <bench-main.hpp>

// This benchmark measures the creation, copying, destruction, and mutation of objects
// It compares DynaMix objects with two C++ alternatives:
// * cpp_class: a class which has all components as members.
//   This is the fastest possible option, but it's not dynamic at all
// * cpp_components: the entity-component pattern: an entity with a vector of
//   individually allocated polymorphic components

//////////////////////////////////////////////////////////////////////////////
// components

struct position { double x = 0, y = 0, z = 0; };
struct velocity { double x = 1, y = 0, z = 0; };
struct health { int hp = 100, max_hp = 100; };
struct nametag { std::string value = "entity"; }; // fits in the small string buffer
struct inventory { std::vector<int> items; };

// big components to compare internal and external mixins
struct big_internal { char data[512] = {}; };
struct big_external { char data[512] = {}; };

constexpr uintptr_t num_components = 5;

//////////////////////////////////////////////////////////////////////////////
// C++ alternatives

struct composite {
    position p;
    velocity v;
    health h;
    nametag n;
    inventory i;
};

struct component_base {
    virtual ~component_base() = default;
    virtual std::unique_ptr<component_base> clone() const = 0;
};

template <typename T>
struct component final : public component_base {
    T data;
    std::unique_ptr<component_base> clone() const override {
        return std::make_unique<component>(*this);
    }
};

struct entity {
    std::vector<std::unique_ptr<component_base>> components;

    entity() {
        components.reserve(8); // be generous to the baseline and avoid reallocations
    }
    entity(entity&&) noexcept = default;
    entity& operator=(entity&&) noexcept = default;

    entity copy() const {
        entity ret;
        for (auto& c : components) {
            ret.components.push_back(c->clone());
        }
        return ret;
    }

    template <typename T>
    void add() {
        components.push_back(std::make_unique<component<T>>());
    }

    template <typename T>
    void remove() {
        auto f = std::find_if(components.begin(), components.end(), [](auto& c) {
            return dynamic_cast<component<T>*>(c.get()) != nullptr;
        });
        if (f != components.end()) components.erase(f);
    }
};

entity make_entity() {
    entity ret;
    ret.add<position>();
    ret.add<velocity>();
    ret.add<health>();
    ret.add<nametag>();
    ret.add<inventory>();
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// DynaMix

struct blife;
DYNAMIX_DECLARE_DOMAIN(blife);
DYNAMIX_DEFINE_DOMAIN(blife);

DYNAMIX_DECLARE_MIXIN(position);
DYNAMIX_DECLARE_MIXIN(velocity);
DYNAMIX_DECLARE_MIXIN(health);
DYNAMIX_DECLARE_MIXIN(nametag);
DYNAMIX_DECLARE_MIXIN(inventory);
DYNAMIX_DECLARE_MIXIN(big_internal);
DYNAMIX_DECLARE_MIXIN(big_external);

DYNAMIX_DEFINE_MIXIN(blife, position);
DYNAMIX_DEFINE_MIXIN(blife, velocity);
DYNAMIX_DEFINE_MIXIN(blife, health);
DYNAMIX_DEFINE_MIXIN(blife, nametag);
DYNAMIX_DEFINE_MIXIN(blife, inventory);
DYNAMIX_DEFINE_MIXIN(blife, big_internal);
DYNAMIX_DEFINE_MIXIN(blife, big_external).force_external();

dynamix::domain& dom() {
    return dynamix::g::get_domain<blife>();
}

void mutate_to_full(dynamix::object& obj) {
    using namespace dynamix;
    mutate(obj, add<position>(), add<velocity>(), add<health>(), add<nametag>(), add<inventory>());
}

const dynamix::type& full_type() {
    dynamix::object obj(dom());
    mutate_to_full(obj);
    return obj.get_type(); // types are owned by the domain and outlive the object
}

template <typename Vec>
uintptr_t count_components(const Vec& vec) {
    uintptr_t ret = 0;
    for (auto& o : vec) {
        if constexpr (std::is_same_v<typename Vec::value_type, dynamix::object>) ret += o.num_mixins();
        else if constexpr (std::is_same_v<typename Vec::value_type, entity>) ret += o.components.size();
        else ret += num_components; // composite
    }
    return ret;
}

std::vector<dynamix::object> make_dynamix_objects(size_t n, dynamix::allocator alloc = {}) {
    auto& t = full_type();
    std::vector<dynamix::object> ret;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        ret.emplace_back(t, alloc);
    }
    return ret;
}

std::vector<std::unique_ptr<composite>> make_composites(size_t n) {
    std::vector<std::unique_ptr<composite>> ret;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        ret.push_back(std::make_unique<composite>());
    }
    return ret;
}

std::vector<entity> make_entities(size_t n) {
    std::vector<entity> ret;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        ret.push_back(make_entity());
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// create
// objects with 5 components
// destruction is not measured

namespace create {

void cpp_class(picobench::state& pb) {
    std::vector<std::unique_ptr<composite>> objs;
    objs.reserve(size_t(pb.iterations()));
    {
        picobench::scope benchmark(pb);
        for (int i = 0; i < pb.iterations(); ++i) {
            objs.push_back(std::make_unique<composite>());
        }
    }
    pb.set_result(count_components(objs));
}

void cpp_components(picobench::state& pb) {
    std::vector<entity> objs;
    objs.reserve(size_t(pb.iterations()));
    {
        picobench::scope benchmark(pb);
        for (int i = 0; i < pb.iterations(); ++i) {
            objs.push_back(make_entity());
        }
    }
    pb.set_result(count_components(objs));
}

// create an empty object and mutate it
// the type already exists, so this includes a type lookup
void dynamix_mutate(picobench::state& pb) {
    full_type(); // make sure the type exists
    std::vector<dynamix::object> objs;
    objs.reserve(size_t(pb.iterations()));
    {
        picobench::scope benchmark(pb);
        for (int i = 0; i < pb.iterations(); ++i) {
            mutate_to_full(objs.emplace_back(dom()));
        }
    }
    pb.set_result(count_components(objs));
}

// create an object from a type: no type lookup
void dynamix_from_type(picobench::state& pb) {
    auto& t = full_type();
    std::vector<dynamix::object> objs;
    objs.reserve(size_t(pb.iterations()));
    {
        picobench::scope benchmark(pb);
        for (int i = 0; i < pb.iterations(); ++i) {
            objs.emplace_back(t);
        }
    }
    pb.set_result(count_components(objs));
}

// create an object from a type with an arena allocator
void dynamix_from_type_arena(picobench::state& pb) {
    auto& t = full_type();
    dbench::arena_resource arena; // must outlive the objects
    std::vector<dynamix::object> objs;
    objs.reserve(size_t(pb.iterations()));
    {
        picobench::scope benchmark(pb);
        for (int i = 0; i < pb.iterations(); ++i) {
            objs.emplace_back(t, dynamix::allocator(&arena));
        }
    }
    pb.set_result(count_components(objs));
}

PICOBENCH_SUITE("create objects");
PICOBENCH(cpp_class).baseline();
PICOBENCH(cpp_components);
PICOBENCH(dynamix_mutate);
PICOBENCH(dynamix_from_type);
PICOBENCH(dynamix_from_type_arena);

} // namespace create

//////////////////////////////////////////////////////////////////////////////
// copy

namespace copy {

void cpp_class(picobench::state& pb) {
    const auto src = make_composites(size_t(pb.iterations()));
    std::vector<std::unique_ptr<composite>> objs;
    objs.reserve(src.size());
    {
        picobench::scope benchmark(pb);
        for (auto& s : src) {
            objs.push_back(std::make_unique<composite>(*s));
        }
    }
    pb.set_result(count_components(objs));
}

void cpp_components(picobench::state& pb) {
    const auto src = make_entities(size_t(pb.iterations()));
    std::vector<entity> objs;
    objs.reserve(src.size());
    {
        picobench::scope benchmark(pb);
        for (auto& s : src) {
            objs.push_back(s.copy());
        }
    }
    pb.set_result(count_components(objs));
}

void dynamix_copy(picobench::state& pb) {
    const auto src = make_dynamix_objects(size_t(pb.iterations()));
    std::vector<dynamix::object> objs;
    objs.reserve(src.size());
    {
        picobench::scope benchmark(pb);
        for (auto& s : src) {
            objs.push_back(s.copy());
        }
    }
    pb.set_result(count_components(objs));
}

PICOBENCH_SUITE("copy objects");
PICOBENCH(cpp_class).baseline();
PICOBENCH(cpp_components);
PICOBENCH(dynamix_copy);

} // namespace copy

//////////////////////////////////////////////////////////////////////////////
// destroy

namespace destroy {

template <typename Vec>
void destroy_benchmark(picobench::state& pb, Vec objs) {
    auto count = count_components(objs);
    {
        picobench::scope benchmark(pb);
        objs.clear();
    }
    pb.set_result(count);
}

void cpp_class(picobench::state& pb) {
    destroy_benchmark(pb, make_composites(size_t(pb.iterations())));
}

void cpp_components(picobench::state& pb) {
    destroy_benchmark(pb, make_entities(size_t(pb.iterations())));
}

void dynamix_destroy(picobench::state& pb) {
    destroy_benchmark(pb, make_dynamix_objects(size_t(pb.iterations())));
}

// with an arena deallocation does nothing
void dynamix_destroy_arena(picobench::state& pb) {
    dbench::arena_resource arena;
    destroy_benchmark(pb, make_dynamix_objects(size_t(pb.iterations()), dynamix::allocator(&arena)));
}

PICOBENCH_SUITE("destroy objects");
PICOBENCH(cpp_class).baseline();
PICOBENCH(cpp_components);
PICOBENCH(dynamix_destroy);
PICOBENCH(dynamix_destroy_arena);

} // namespace destroy

//////////////////////////////////////////////////////////////////////////////
// add and remove a component
// objects have 4 components, and in each iteration a fifth one is added and then removed
// for DynaMix the types are cached, so this measures two mutations with type lookups

namespace add_remove {

void cpp_components(picobench::state& pb) {
    std::vector<entity> objs(size_t(pb.iterations()));
    for (auto& e : objs) {
        e.add<position>();
        e.add<velocity>();
        e.add<health>();
        e.add<nametag>();
    }

    uintptr_t count = 0;
    {
        picobench::scope benchmark(pb);
        for (auto& e : objs) {
            e.add<inventory>();
            count += e.components.size();
            e.remove<inventory>();
        }
    }
    pb.set_result(count);
}

void dynamix_benchmark(picobench::state& pb, dynamix::allocator alloc) {
    using namespace dynamix;
    std::vector<object> objs;
    objs.reserve(size_t(pb.iterations()));
    for (int i = 0; i < pb.iterations(); ++i) {
        auto& o = objs.emplace_back(dom(), alloc);
        mutate(o, add<position>(), add<velocity>(), add<health>(), add<nametag>());
    }
    // make sure the types exist
    mutate(objs.front(), add<inventory>());
    mutate(objs.front(), remove<inventory>());

    uintptr_t count = 0;
    {
        picobench::scope benchmark(pb);
        for (auto& o : objs) {
            mutate(o, add<inventory>());
            count += o.num_mixins();
            mutate(o, remove<inventory>());
        }
    }
    pb.set_result(count);
}

void dynamix_mutate(picobench::state& pb) {
    dynamix_benchmark(pb, {});
}

// a mutation deallocates the old object buffer and allocates a new one
// with a recycling resource the buffers are reused
void dynamix_mutate_recycling(picobench::state& pb) {
    dbench::recycling_resource res; // must outlive the objects
    dynamix_benchmark(pb, dynamix::allocator(&res));
}

PICOBENCH_SUITE("add and remove a component");
PICOBENCH(cpp_components).baseline();
PICOBENCH(dynamix_mutate);
PICOBENCH(dynamix_mutate_recycling);

} // namespace add_remove

//////////////////////////////////////////////////////////////////////////////
// mutations of objects with a big mixin
// objects have position and a 512-byte mixin and a third small one is added and removed
// internal mixins are moved to the new buffer on every mutation, external ones are not

namespace big {

template <typename Big>
void big_benchmark(picobench::state& pb) {
    using namespace dynamix;
    std::vector<object> objs;
    objs.reserve(size_t(pb.iterations()));
    for (int i = 0; i < pb.iterations(); ++i) {
        mutate(objs.emplace_back(dom()), add<position>(), add<Big>());
    }
    mutate(objs.front(), add<health>());
    mutate(objs.front(), dynamix::remove<health>());

    uintptr_t count = 0;
    {
        picobench::scope benchmark(pb);
        for (auto& o : objs) {
            mutate(o, add<health>());
            count += o.num_mixins();
            mutate(o, dynamix::remove<health>());
        }
    }
    pb.set_result(count);
}

void internal(picobench::state& pb) {
    big_benchmark<big_internal>(pb);
}

void external(picobench::state& pb) {
    big_benchmark<big_external>(pb);
}

PICOBENCH_SUITE("mutations with a 512-byte mixin");
PICOBENCH(internal).baseline();
PICOBENCH(external);

} // namespace big

int main(int argc, char* argv[]) {
    return dbench::run(argc, argv, {1024, 65536});
}
