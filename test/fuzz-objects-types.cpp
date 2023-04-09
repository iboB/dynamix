#include "test_mixin_allocator.hpp"

#include <dynamix/domain.hpp>
#include <dynamix/feature_info_data.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/exception.hpp>

#include <doctest/doctest.h>

#include <itlib/qalgorithm.hpp>
#include <itlib/flat_set.hpp>

#include <thread>
#include <deque>
#include <cstdlib>
#include <random>
#include <array>

static constexpr int NUM_FEATURES = 20;
static constexpr int NUM_DEPS = 4;
static constexpr int NUM_MIXINS = 10;
static constexpr int SIZE = 100;

struct object_producer {
    dynamix::domain& dom;
    const std::deque<dynamix::util::mixin_info_data>& mixins;
    std::minstd_rand rnd;
    std::vector<dynamix::object> objects;

    object_producer(dynamix::domain& d, const std::deque<dynamix::util::mixin_info_data>& mix, uint32_t seed)
        : dom(d)
        , mixins(mix)
        , rnd(seed)
    {}

    void produce() {
        // intentionally not reserving
        // we want the vector to move things around
        while (objects.size() != SIZE) {
            auto num_mixins = rnd() % 10 + 1;

            itlib::flat_set<const dynamix::mixin_info*> mix;
            while (mix.size() != num_mixins) mix.insert(&mixins[rnd() % mixins.size()].info); // generate unique infos
            auto mix_shuf = std::move(mix.modify_container());
            std::shuffle(mix_shuf.begin(), mix_shuf.end(), rnd); // but not sorted

            try {
                auto& t = dom.get_type(mix_shuf);
                auto& o = objects.emplace_back(dom);
                o.reset_type(t);
            }
            catch (const dynamix::exception&) {
                continue;
            }
        }
    }
};

TEST_CASE("fuzz objects and types") {
    std::random_device rd;
    std::minstd_rand rnd(rd());

    std::deque<dynamix::util::feature_info_data> features;
    // generate features
    for (int i = 0; i < NUM_FEATURES; ++i) {
        auto& f = features.emplace_back();
        dynamix::util::feature_info_data_builder b(f, "");
        auto name = "feature_" + std::to_string(i);
        b.store_name(name);

        auto op = rnd() % 3;
        if (op == 0) {
            b.default_payload_by(name);
        }
        else if (op == 1) {
            b.default_payload_with(std::pmr::string("fff"));
        }

        f.info.feature_class = rnd() % 5;
        f.info.allow_clashes = rnd() % 20 != 0; // allow clashes 95% of the time
    }

    test_mixin_allocator alloc;
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

        // funcs
        {
            // override init funcs from builder
            info.init = dnmx_mixin_common_init_func;
            info.move_init = dnmx_mixin_common_move_func;
            info.move_asgn = dnmx_mixin_common_move_func;
            info.copy_init = dnmx_mixin_common_copy_func;
            info.copy_asgn = dnmx_mixin_common_copy_func;
            info.compare = dnmx_mixin_common_cmp_func;
            info.destroy = dnmx_mixin_common_destroy_func;

            if (!dep && (rnd() % 5 == 0)) {
                // no default ctor 20% of the time
                // but only if not a dependency
                info.init = nullptr;
            }

            if (rnd() % 20 == 0) {
                // no move 5% of the time
                info.move_init = nullptr;
                info.move_asgn = nullptr;
            }

            if (rnd() % 4 == 0) {
                // no copy 25% of the time
                info.copy_init = nullptr;
                info.copy_asgn = nullptr;
            }

            if (rnd() % 2 == 0) {
                // no compare 50% of the time
                info.compare = nullptr;
                if (rnd() % 2) {
                    // ... but have an equals func 25% of the time
                    info.equals = dnmx_mixin_common_eq_func;
                }
            }

            if (rnd() % 2 == 0) {
                // no destroy 50% of the time
                info.destroy = nullptr;
            }
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
            auto max_features = dep ? 3 : 13;
            auto num_features = rnd() % max_features;
            itlib::flat_set<uint32_t> fids;
            while (fids.size() != num_features) fids.insert(rnd() % NUM_FEATURES); // generate unique indices
            auto fids_shuf = std::move(fids.modify_container());
            std::shuffle(fids_shuf.begin(), fids_shuf.end(), rnd); // but not sorted

            for (auto fid : fids_shuf) {
                auto& name = m.stored_name;
                auto& f = features[fid];

                dynamix::util::builder_perks perks;
                perks.bid = rnd() % 3 - 1;
                perks.priority = rnd() % 3 - 1;

                auto op = rnd() % 2;
                if (op == 0) {
                    std::pmr::string ffm = name + '-' + f.stored_name;
                    ffm += f.info.name.to_std();
                    b.implements_with(f.info, ffm, perks);
                }
                else {
                    b.implements_by(f.info, name, perks);
                }
            }
        }

        // other

        if (rnd() % 5 == 0) {
            // custom allocator 20% of the time
            if (rnd() % 2) {
                b.uses_allocator(alloc);
            }
            else {
                b.uses_allocator<test_mixin_allocator>();
            }
        }

        info.mixin_class = rnd() % 10;
        info.type_info_class = rnd() % 20;
        info.user_data = rnd() % 100;

        info.force_external = rnd() % 50 == 0; // force external 2% of the time
    };

    // generate deps
    std::deque<dynamix::util::mixin_info_data> deps;
    for (int i = 0; i < NUM_DEPS; ++i) {
        auto& m = deps.emplace_back();
        create_mixin(m, i, true);
    }

    // generate mixins
    std::deque<dynamix::util::mixin_info_data> mixins;
    for (int i = 0; i < NUM_MIXINS; ++i) {
        auto& m = mixins.emplace_back();
        create_mixin(m, i, false);
    }

    // add rules
    for (auto& d : deps) {
        auto num = rnd() % 2 + 1;
        for (uint32_t i = 0; i < num; ++i) {
            auto to = rnd() % NUM_MIXINS;
            auto r = d.mutation_rule_info_storage.emplace_back();
            auto& m = mixins[to];
            r.apply = [](dnmx_type_mutation_handle mutation, uintptr_t user_data) {
                auto mut = dynamix::type_mutation::from_c_handle(mutation);
                auto name = std::string_view(reinterpret_cast<const char*>(user_data));
                mut->add_if_lacking(name);
                return dynamix::result_success;
            };
            r.name = m.info.name;
            r.user_data = reinterpret_cast<uintptr_t>(m.stored_name.c_str());
            d.mutation_rule_infos.push_back(&r);
        }
    }

    //dynamix::domain dom;
    //for (auto& m : mixins) {
    //    m.register_in(dom);
    //}
    //for (auto& d : deps) {
    //    d.register_in(dom);
    //}

    //std::deque<object_producer> producers;
    //for (int i = 0; i < 1; ++i) {
    //    producers.emplace_back(dom, mixins, rd());
    //}

    //std::vector<std::thread> threads;
    //for (auto& p : producers) {
    //    threads.emplace_back([&]() { p.produce(); });
    //}

    //for (auto& t : threads) {
    //    t.join();
    //}
}
