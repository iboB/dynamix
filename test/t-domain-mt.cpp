// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <doctest/doctest.h>

#include <thread>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("empty") {
    std::vector<std::string> strings;
    static constexpr dnmx_id_int_t TSIZE = 20;
    for (dnmx_id_int_t i = 0; i < TSIZE; ++i) {
        strings.push_back("s" + std::to_string(i));
    }

    domain dom;

    std::thread feature_by_name([&]() {
        for (auto& s : strings) {
            CHECK_FALSE(dom.get_feature_info(s));
        }
    });
    std::thread mixin_by_name([&]() {
        for (auto& s : strings) {
            CHECK_FALSE(dom.get_mixin_info(s));
        }
    });
    std::thread feature_by_id([&]() {
        for (dnmx_id_int_t i = 0; i < TSIZE; ++i) {
            CHECK_FALSE(dom.get_feature_info(feature_id{i}));
        }
    });
    std::thread mixin_by_id([&]() {
        for (dnmx_id_int_t i = 0; i < TSIZE; ++i) {
            CHECK_FALSE(dom.get_mixin_info(mixin_id{i}));
        }
    });
    feature_by_name.join();
    mixin_by_name.join();
    feature_by_id.join();
    mixin_by_id.join();
}

TEST_CASE("features") {
    domain dom;
    test_data t;
    auto& features = t.features;
    auto mid = features.begin() + features.size() / 2;

    std::thread first_half([&]() {
        for (auto i = features.begin(); i != mid; ++i) {
            dom.register_feature(*i);
        }
    });
    std::thread second_half([&]() {
        for (auto i = mid; i != features.end(); ++i) {
            dom.register_feature(*i);
        }
    });
    std::thread q_by_name([&]() {
        for (auto& f : features) {
            auto q = dom.get_feature_info(f.name.to_std());
            CHECK((q == nullptr || q == &f));
        }
    });
    first_half.join();
    second_half.join();
    q_by_name.join();

    for (auto& f : features) {
        CHECK(&f == dom.get_feature_info(f.name.to_std()));
    }
}

TEST_CASE("mixins") {
    domain dom;
    test_data t;
    auto& mixins = t.mixins;
    auto mid = mixins.begin() + mixins.size() / 2;

    std::thread first_half([&]() {
        for (auto i = mixins.begin(); i != mid; ++i) {
            dom.register_mixin(i->info);
        }
    });
    std::thread second_half([&]() {
        for (auto i = mid; i != mixins.end(); ++i) {
            dom.register_mixin(i->info);
        }
    });
    std::thread q_by_name([&]() {
        for (auto& m : mixins) {
            auto q = dom.get_mixin_info(m.info.name.to_std());
            CHECK((q == nullptr || q == &m.info));
        }
    });
    first_half.join();
    second_half.join();
    q_by_name.join();

    for (auto& m : mixins) {
        CHECK(&m.info == dom.get_mixin_info(m.info.name.to_std()));
    }
}
