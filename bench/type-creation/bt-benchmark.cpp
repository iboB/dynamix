// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/domain.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/feature_info_data.hpp>
#include <dynamix/type_mutation.hpp>

#include <bench-rand.hpp>

#include <memory>
#include <vector>
#include <set>
#include <array>

#define PICOBENCH_IMPLEMENT
#include <bench-main.hpp>

enum bench_kind : uintptr_t {
    bench_create,
    bench_lookup,
};

// some code pasted from fuzz-objects-types.cpp

class custom_rule {
    dynamix::mutation_rule_info m_info;
    const dynamix::mixin_info& m_primary;
    const dynamix::mixin_info& m_dep;
    std::string m_name;
public:
    custom_rule(const dynamix::mixin_info& primary, const dynamix::mixin_info& dep)
        : m_primary(primary)
        , m_dep(dep)
    {
        m_name = "r ";
        m_name += m_dep.name.to_std();
        m_name += " to ";
        m_name += m_primary.name.to_std();

        m_info.apply = apply;
        m_info.name = dnmx_sv::from_std(m_name);
        m_info.order_priority = 0;
        m_info.user_data = reinterpret_cast<uintptr_t>(this);
    }

    static dynamix::error_return_t apply(dnmx_type_mutation_handle mutation, uintptr_t user_data) {
        auto mut = dynamix::type_mutation::from_c_handle(mutation);
        auto self = reinterpret_cast<custom_rule*>(user_data);
        if (!mut->has(self->m_primary)) return dynamix::result_success;
        mut->add_if_lacking(self->m_dep);
        return dynamix::result_success;
    }

    custom_rule(const custom_rule&) = delete;
    custom_rule& operator=(const custom_rule&) = delete;

    const dynamix::mutation_rule_info& info() const noexcept { return m_info; };
};

constexpr int NUM_MIXINS = 10;

struct state {
    std::vector<std::unique_ptr<dynamix::util::feature_info_data>> features;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> deps;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> mixins;
    std::vector<std::unique_ptr<custom_rule>> rules;
};

state create_state(dynamix::domain& dom, uint32_t features_per_mixin, uint32_t num_rules, std::minstd_rand& rnd) {
    state ret;
    for (uint32_t i = 0; i < features_per_mixin * 10; ++i) {
        auto& f = ret.features.emplace_back(new dynamix::util::feature_info_data);
        dynamix::util::feature_info_data_builder b(*f, "");
        auto name = "feature_" + std::to_string(i);
        b.store_name(name);
        f->info.allow_clashes = true;
    }

    auto create_mixin = [&](dynamix::util::mixin_info_data& m, int i, bool dep) {
        auto& info = m.info;
        dynamix::util::mixin_info_data_builder b(m, "");
        if (dep) {
            b.store_name("dep_" + std::to_string(i));
            b.dependency();
        }
        else {
            b.store_name("mixin_" + std::to_string(i));
        }

        // size, alignment
        {
            static constexpr std::array<dynamix::byte_size_t, 7> alignments = {0, 1, 2, 4, 8, 16, 32};
            auto alignment = alignments[rnd() % alignments.size()];
            auto size = alignment * (rnd() % 10 + 1);
            info.set_size_alignment(size, alignment);
        }

        // features
        {
            auto fids = dbench::pick_unique(features_per_mixin, uint32_t(ret.features.size()), rnd);
            for (auto fid : fids) {
                auto& f = *ret.features[fid];

                dynamix::util::builder_perks perks;
                perks.bid = rnd() % 3 - 1;
                perks.priority = rnd() % 3 - 1;

                dynamix::compat::pmr::string ffm = m.stored_name + '-' + f.stored_name;
                ffm += f.info.name.to_std();
                b.implements_with(f.info, ffm, perks);
            }
        }

        info.force_external = rnd() % 10 == 0; // force external 10% of the time

        if (dep) {
            auto to = rnd() % ret.mixins.size();
            auto& prim = ret.mixins[to]->info;
            auto& r = *ret.rules.emplace_back(new custom_rule(prim, m.info));
            b.adds_mutation_rule(r.info());
        }

        m.register_in(dom);
    };

    for (int i = 0; i < NUM_MIXINS; ++i) {
        auto& m = *ret.mixins.emplace_back(new dynamix::util::mixin_info_data);
        create_mixin(m, i, false);
    }

    for (int i = 0; i < int(num_rules); ++i) {
        auto& m = *ret.deps.emplace_back(new dynamix::util::mixin_info_data);
        create_mixin(m, i, true);
    }

    return ret;
}

// generate n unique type queries, each with `mixins` unique mixins in random order
// since types are ordered, queries with the same mixins in different orders are different types
// there are NUM_MIXINS! / (NUM_MIXINS - mixins)! possible queries (30240 for 5 of 10)
std::vector<std::vector<uint32_t>> make_unique_queries(size_t n, uint32_t mixins, dbench::rng_t& rng) {
    std::set<std::vector<uint32_t>> unique;
    std::vector<std::vector<uint32_t>> ret;
    ret.reserve(n);
    while (ret.size() != n) {
        auto q = dbench::pick_unique(mixins, NUM_MIXINS, rng);
        if (unique.insert(q).second) {
            ret.push_back(std::move(q));
        }
    }
    return ret;
}

struct bench_params {
    uint32_t mixins;
    uint32_t features;
    uint32_t rules;
};

struct test_data {
    state st; // declared before the domain so it's destroyed after it
    dynamix::domain dom;
    std::vector<std::vector<const dynamix::mixin_info*>> queries;

    test_data(const bench_params& p, size_t n)
        : dom("bench")
    {
        dbench::rng_t rng(13);

        // first create queries by index while the rng is not tainted by create_state
        auto indices = make_unique_queries(n, p.mixins, rng);

        st = create_state(dom, p.features, p.rules, rng);

        queries.reserve(n);
        for (auto& is : indices) {
            auto& q = queries.emplace_back();
            q.reserve(is.size());
            for (auto index : is) {
                q.push_back(&st.mixins[index]->info);
            }
        }
    }
};

// every query is unique and the domain is new, so every get_type creates a new type
// this includes applying mutation rules and building the feature table
// in practice this happens once per unique type in a domain
void create_types(const bench_params& p, picobench::state& pb) {
    test_data data(p, size_t(pb.iterations()));

    {
        dbench::scope benchmark(pb);
        for (auto& q : data.queries) {
            data.dom.get_type(q);
        }
    }
    pb.set_result(data.dom.num_types());
}

// all types are created before the measured part, so every get_type is a lookup of an existing type
// this is what happens in the vast majority of mutations
void lookup_types(const bench_params& p, picobench::state& pb) {
    test_data data(p, size_t(pb.iterations()));

    std::vector<const dynamix::type*> types;
    types.reserve(data.queries.size());
    for (auto& q : data.queries) {
        types.push_back(&data.dom.get_type(q));
    }

    // look the types up in a different order than the one in which they were created
    dbench::rng_t rng(42);
    const auto order = dbench::make_access_order(data.queries.size(), true, rng);

    uintptr_t found = 0;
    {
        dbench::scope benchmark(pb);
        for (auto i : order) {
            auto& t = data.dom.get_type(data.queries[i]);
            found += &t == types[i];
        }
    }
    pb.set_result(found);
}

constexpr bench_params trivial_mixins_params = {5, 0, 0};
constexpr bench_params many_rules_params = {5, 0, 10};
constexpr bench_params many_features_params = {5, 20, 0};

void trivial_mixins(picobench::state& pb) {
    if (pb.benchmark_user_data() == bench_create) create_types(trivial_mixins_params, pb);
    else lookup_types(trivial_mixins_params, pb);
}

void many_rules(picobench::state& pb) {
    if (pb.benchmark_user_data() == bench_create) create_types(many_rules_params, pb);
    else lookup_types(many_rules_params, pb);
}

void many_features(picobench::state& pb) {
    if (pb.benchmark_user_data() == bench_create) create_types(many_features_params, pb);
    else lookup_types(many_features_params, pb);
}

PICOBENCH_SUITE("type creation");
PICOBENCH(trivial_mixins).user_data(bench_create).baseline();
PICOBENCH(many_rules).user_data(bench_create);
PICOBENCH(many_features).user_data(bench_create);

PICOBENCH_SUITE("type lookup");
PICOBENCH(trivial_mixins).user_data(bench_lookup).baseline();
PICOBENCH(many_rules).user_data(bench_lookup);
PICOBENCH(many_features).user_data(bench_lookup);

int main(int argc, char* argv[]) {
    return dbench::run(argc, argv, {128, 1024, 16384});
}
