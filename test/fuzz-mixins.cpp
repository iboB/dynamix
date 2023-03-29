#include <dynamix/domain.hpp>
#include <dynamix/feature_info_data.hpp>
#include <dynamix/mixin_info_data.hpp>

#include <doctest/doctest.h>
#include <itlib/qalgorithm.hpp>
#include <thread>
#include <deque>
#include <cstdlib>
#include <random>

static constexpr int NUM_FEATURES = 200;
static constexpr int SIZE = 1000;

struct mixin_generator {
    dynamix::domain& dom;
    std::string base_name;
    std::minstd_rand rnd;
    const std::deque<dynamix::util::feature_info_data>& features;
    std::deque<dynamix::util::mixin_info_data> mixins;

    mixin_generator(dynamix::domain& d, std::string_view name, const std::deque<dynamix::util::feature_info_data>& features, uint32_t seed)
        : dom(d)
        , features(features)
        , base_name(name)
        , rnd(seed)
    {}

    void reg_mixin() {
        std::string name = base_name + std::to_string(mixins.size());

        auto& m = mixins.emplace_back();
        dynamix::util::mixin_info_data_builder b(m, "");
        b.store_name(name);

        auto num_features = rnd() % 10;
        for (uint32_t j = 0; j < num_features; ++j) {
            auto feature_index = rnd() % NUM_FEATURES;
            auto& f = features[feature_index];

            auto op = rnd() % 2;
            if (op == 0) {
                std::string ffm = name;
                ffm += f.info.name.to_std();
                b.implements_with(f.info, ffm);
            }
            else {
                b.implements_by(f.info, base_name);
            }
        }

        dom.register_mixin(m.info);
    }

    void unreg_mixin() {
        if (mixins.empty()) return; // nothing to unregister
        auto mixin_index = rnd() % mixins.size();
        auto& m = mixins[mixin_index];
        if (!m.info.registered()) return; // already unregistered, so do nothing
        dom.unregister_mixin(m.info);
    }

    void generate() {
        for (int i = 0; i < SIZE; ++i) {
            auto op = rnd() % 10;
            if (op == 0) {
                // unregisterd one in 10 times
                unreg_mixin();
            }
            else {
                reg_mixin();
            }
        }
    }
};

struct mixin_getter {
    dynamix::domain& dom;
    const std::vector<std::string>& name_pool;
    std::minstd_rand rnd;

    mixin_getter(dynamix::domain& d, std::vector<std::string>& names, uint32_t seed)
        : dom(d)
        , name_pool(names)
        , rnd(seed)
    {}

    void get() {
        int successes = 0;
        while (successes != SIZE) {
            const dynamix::mixin_info* m = nullptr;

            if (rnd() % 2) {
                // get by id
                dnmx_id_int_t id = dnmx_id_int_t(rnd() % (name_pool.size() * SIZE));
                m = dom.get_mixin_info(dynamix::mixin_id{id});
            }
            else {
                // get by name
                auto name = name_pool[rnd() % name_pool.size()];
                name += std::to_string(rnd() % SIZE);
                m = dom.get_mixin_info(name);
            }

            if (!m) continue;
            ++successes;

            std::string name{m->name.to_std()};
            auto fspan = m->features_span();
            for (auto& f : fspan) {
                auto pl = reinterpret_cast<std::string*>(f.payload);
                REQUIRE(pl);
                CHECK((*pl == name + std::string(f.info->name.to_std()) || itlib::pfind(name_pool, *pl)));
            }
        }
    }
};

TEST_CASE("fuzz mixins") {
    std::deque<dynamix::util::feature_info_data> features;
    // generate features
    for (int i = 0; i < NUM_FEATURES; ++i) {
        auto& f = features.emplace_back();
        dynamix::util::feature_info_data_builder b(f, "");
        b.store_name("feature_" + std::to_string(i));
    }

    std::vector<std::string> bases = {
        "mixin_a",
        "mb_",
        "a rather long base mixin name, if I might say so "
    };

    std::deque<mixin_generator> generators;
    std::deque<mixin_getter> getters;

    std::random_device rd;

    dynamix::domain dom;
    for (auto& b : bases) {
        generators.emplace_back(dom, b, features, rd());
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