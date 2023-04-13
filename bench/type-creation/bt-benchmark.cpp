// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/domain.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/feature_info_data.hpp>
#include <dynamix/type_mutation.hpp>

#include <picobench/picobench.hpp>

#include <memory>
#include <vector>
#include <unordered_set>
#include <random>
#include <array>

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

        // size, aligment
        {
            static constexpr std::array<dynamix::byte_size_t, 7> alignments = {0, 1, 2, 4, 8, 16, 32};
            auto alignment = alignments[rnd() % alignments.size()];
            auto size = alignment * (rnd() % 10 + 1);
            info.set_size_alignment(size, alignment);
        }

        // features
        {
            std::unordered_set<uint32_t> fids;
            while (fids.size() != features_per_mixin) fids.insert(rnd() % ret.features.size()); // generate unique indices
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

void benchmark(uint32_t mixins, uint32_t features, uint32_t rules, picobench::state& pb) {
    dynamix::domain dom("bench");
    std::minstd_rand rnd(13);

    // first create types by index while the rng is not tainted by create_state
    std::vector<std::vector<uint32_t>> indices(pb.iterations());
    for (auto& is : indices) {
        auto num_mixins = mixins;
        std::unordered_set<uint32_t> mids;
        while (mids.size() != num_mixins) {
            mids.insert(rnd() % NUM_MIXINS);
        }
        is.assign(mids.begin(), mids.end());
    }

    auto state = create_state(dom, features, rules, rnd);

    std::vector<std::vector<const dynamix::mixin_info*>> types;
    types.reserve(pb.iterations());

    for (auto& is : indices) {
        auto& t = types.emplace_back();
        t.reserve(is.size());
        for (auto index : is) {
            t.push_back(&state.mixins[index]->info);
        }
    }

    picobench::scope benchmark(pb);
    for (auto& t : types) {
        dom.get_type(t);
    }
    pb.set_result(dom.num_types());
}

void trivial_mixins(picobench::state& pb) {
    benchmark(5, 0, 0, pb);
}
PICOBENCH(trivial_mixins).baseline();

void many_rules(picobench::state& pb) {
    benchmark(5, 0, 10, pb);
}
PICOBENCH(many_rules);

void many_features(picobench::state& pb) {
    benchmark(5, 20, 0, pb);
}
PICOBENCH(many_features);
