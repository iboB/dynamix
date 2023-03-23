// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/domain.hpp>
#include <dynamix/feature_info_data.hpp>

#include <doctest/doctest.h>
#include <thread>
#include <deque>
#include <cstdlib>
#include <random>

static constexpr int SIZE = 1000;

struct feature_generator {
    dynamix::domain& dom;
    std::string base_name;
    std::minstd_rand rnd;
    std::deque<dynamix::util::feature_info_data> features;

    feature_generator(dynamix::domain& d, std::string_view name, uint32_t seed)
        : dom(d)
        , base_name(name)
        , rnd(seed)
    {}

    void generate() {
        for (int i = 0; i < SIZE; ++i) {
            std::string name = base_name + std::to_string(features.size());

            auto& f = features.emplace_back();
            dynamix::util::feature_info_data_builder b(f, "");
            b.store_name(name);

            auto op = rnd() % 3;
            if (op == 0) {
                b.default_payload_by(base_name);
            }
            else if (op == 1) {
                b.default_payload_with(name);
            }
            // no default payload on op == 2

            dom.register_feature(f.info);
        }
    }
};

struct feature_getter {
    dynamix::domain& dom;
    const std::vector<std::string>& name_pool;
    std::minstd_rand rnd;

    feature_getter(dynamix::domain& d, std::vector<std::string>& names, uint32_t seed)
        : dom(d)
        , name_pool(names)
        , rnd(seed)
    {}

    void get() {
        int successes = 0;
        while (successes != SIZE) {
            const dynamix::feature_info* f = nullptr;

            if (rnd() % 2) {
                // get by id
                dnmx_id_int_t id = dnmx_id_int_t(rnd() % (name_pool.size() * SIZE));
                f = dom.get_feature_info(dynamix::feature_id{id});
            }
            else {
                // get by name
                auto name = name_pool[rnd() % name_pool.size()];
                name += std::to_string(rnd() % SIZE);
                f = dom.get_feature_info(name);
            }

            if (!f) continue;
            ++successes;

            auto name = f->name.to_std();
            CHECK(name.length() > 1);
            CHECK(name.length() < 100);

            auto pl = reinterpret_cast<std::string*>(f->default_payload);
            if (!pl) continue;
            CHECK(pl->length() > 1);
            CHECK(pl->length() < 100);
        }
    }
};

TEST_CASE("fuzz features") {
    dynamix::domain dom;

    std::vector<std::string> bases = {
        "feature_a_",
        "fb_",
        "a rather long base feature name, if I might say so "
    };

    std::deque<feature_generator> generators;
    std::deque<feature_getter> getters;

    std::random_device rd;
    for (auto& b : bases) {
        generators.emplace_back(dom, b, rd());
    }

    for (int i = 0; i < 2; ++i) {
        getters.emplace_back(dom, bases, rd());
    }

    std::vector<std::thread> threads;
    for (auto& g : getters) {
        threads.emplace_back([&]() { g.get(); });
    }
    for (auto& g : generators) {
        threads.emplace_back([&]() { g.generate(); });
    }

    for (auto& t : threads) {
        t.join();
    }
}
