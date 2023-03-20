// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_mixin_allocator.hpp"

#include <dynamix/mixin_info_data.hpp>
#include <dynamix/common_feature_info.hpp>

#include <doctest/util/lifetime_counter.hpp>

using namespace dynamix;
using namespace dynamix::util;
using namespace dynamix::util::builder_literals;

struct test_mixin {
    int x;
    int y;
    static int fpayload;
};
int test_mixin::fpayload = 8;

TEST_CASE("mixin_info_data build name") {
    mixin_info_data data;
    mixin_info_data_builder<void> b(data, "void");
    CHECK(&data == &b.get_data());
    const auto& info = data.info;
    CHECK(info.name.to_std() == "void");
    CHECK(info.size == 0);
    CHECK(info.alignment == 0);
    CHECK(!info.destroy);

    b.store_name("bar");
    CHECK(info.name.to_std() == "bar");
    CHECK(data.stored_name == "bar");
    CHECK((const void*)data.stored_name.c_str() == info.name.begin);

    b.name("xxx");
    CHECK((const void*)data.stored_name.c_str() != info.name.begin);
    CHECK(info.name.to_std() == "xxx");
}

TEST_CASE("mixin_info_data build allocator") {
    mixin_info_data data;
    mixin_info_data_builder<test_mixin> b(data, "test");
    const auto& info = data.info;
    CHECK(info.name.to_std() == "test");

    test_mixin_allocator alloc;
    b.uses_allocator(alloc);
    CHECK(info.allocator == alloc.to_c_hanlde());
    CHECK(info.allocator == data.stored_alloc->to_c_hanlde());

    b.uses_allocator<test_mixin_allocator>();
    CHECK(info.allocator != alloc.to_c_hanlde());
    CHECK(info.allocator == data.stored_alloc->to_c_hanlde());

    b.uses_allocator(alloc);
    CHECK(info.allocator == alloc.to_c_hanlde());
    CHECK(info.allocator == data.stored_alloc->to_c_hanlde());
}

TEST_CASE("mixin_info_data build mutation rules") {
    mixin_info_data data;
    mixin_info_data_builder<test_mixin> b(data, "test");
    mutation_rule_info rule = {};
    b.adds_mutation_rule(rule);
    CHECK(data.mutation_rule_infos.size() == 1);
    CHECK(data.mutation_rule_infos.front() == &rule);
    CHECK(data.mutation_rule_info_storage.empty());

    b.adds_mutation_rule([](dnmx_type_mutation_handle, uintptr_t) { return 125; });

    CHECK(data.mutation_rule_infos.size() == 2);
    CHECK(data.mutation_rule_infos.front() == &rule);
    CHECK(data.mutation_rule_info_storage.size() == 1);
    CHECK(data.mutation_rule_infos[1] == &data.mutation_rule_info_storage[0]);

    {
        auto& crule = data.mutation_rule_info_storage[0];
        CHECK(crule.name.empty());
        CHECK(crule.apply(nullptr, 0) == 125);
        CHECK(crule.order_priority == 0);
        CHECK(crule.user_data == 0);
    }

    b.adds_mutation_rule([](dnmx_type_mutation_handle, uintptr_t) { return 42; }, 45, -6_prio);

    CHECK(data.mutation_rule_infos.size() == 3);
    CHECK(data.mutation_rule_infos.front() == &rule);
    CHECK(data.mutation_rule_info_storage.size() == 2);
    CHECK(data.mutation_rule_infos[1] == &data.mutation_rule_info_storage[0]);
    CHECK(data.mutation_rule_infos[2] == &data.mutation_rule_info_storage[1]);

    {
        auto& crule = data.mutation_rule_info_storage[0];
        CHECK(crule.name.empty());
        CHECK(crule.apply(nullptr, 0) == 125);
        CHECK(crule.order_priority == 0);
        CHECK(crule.user_data == 0);
    }

    {
        auto& crule = data.mutation_rule_info_storage[1];
        CHECK(crule.name.empty());
        CHECK(crule.apply(nullptr, 0) == 42);
        CHECK(crule.order_priority == -6);
        CHECK(crule.user_data == 45);
    }
}

struct complex_feature {
    struct traits {
        template <typename Mixin>
        static any make_payload_for(const allocator&, Mixin*) {
            return fwd_any(test_mixin::fpayload);
        }
        static any make_payload_by(const allocator&, void*, int& pl) {
            return fwd_any(pl);
        }
        static any make_payload_with(const allocator& alloc, void*, int pl) {
            return make_any(alloc, pl);
        }
    };
    /* feature info */
    static const common_feature_info& get_info_safe() {
        static common_feature_info the_info(dnmx_make_sv_lit("complex"));
        return the_info;
    }
    static const common_feature_info& info;
};
const common_feature_info& complex_feature::info = complex_feature::get_info_safe();

TEST_CASE("mixin_info_data build features") {
    common_feature_info simple_feature(dnmx_make_sv_lit("simple feature"));

    mixin_info_data data;
    mixin_info_data_builder<test_mixin> b(data, "test");
    const auto& info = data.info;

    int pl0 = 5;
    b.implements_by(simple_feature, &pl0, order_priority(6), 5_bid);
    CHECK(info.num_features == 1);
    CHECK(data.feature_payloads.size() == 1);
    CHECK(info.features == data.feature_payloads.data());

    {
        auto& ffm = info.features[0];
        CHECK(ffm.info == &simple_feature);
        CHECK(ffm.payload == &pl0);
        CHECK(ffm.bid == 5);
        CHECK(ffm.priority == 6);
    }

    b.implements_with(simple_feature, std::string("foo"), feature_bid_priority(4, -10));
    CHECK(info.num_features == 2);
    CHECK(data.feature_payloads.size() == 2);
    CHECK(info.features == data.feature_payloads.data());
    CHECK(data.feature_payload_storage.size() == 2);

    {
        auto& ffm = info.features[1];
        CHECK(ffm.info == &simple_feature);
        CHECK(*static_cast<std::string*>(ffm.payload) == "foo");
        CHECK(ffm.bid == 4);
        CHECK(ffm.priority == -10);
    }

    b.implements<complex_feature>();
    CHECK(info.num_features == 3);
    CHECK(data.feature_payloads.size() == 3);
    CHECK(info.features == data.feature_payloads.data());
    CHECK(data.feature_payload_storage.size() == 3);

    {
        auto& ffm = info.features[2];
        CHECK(ffm.info == &complex_feature::info);
        CHECK(ffm.payload == &test_mixin::fpayload);
        CHECK(ffm.bid == 0);
        CHECK(ffm.priority == 0);
    }

    b.implements_with<complex_feature>(45, feature_bid(5));
    CHECK(info.num_features == 4);
    CHECK(data.feature_payloads.size() == 4);
    CHECK(info.features == data.feature_payloads.data());
    CHECK(data.feature_payload_storage.size() == 4);

    {
        auto& ffm = info.features[3];
        CHECK(ffm.info == &complex_feature::info);
        CHECK(*static_cast<int*>(ffm.payload) == 45);
        CHECK(ffm.bid == 5);
        CHECK(ffm.priority == 0);
    }

    int pl1 = 123;
    b.implements_by<complex_feature>(pl1, -3_prio);
    CHECK(info.num_features == 5);
    CHECK(data.feature_payloads.size() == 5);
    CHECK(info.features == data.feature_payloads.data());
    CHECK(data.feature_payload_storage.size() == 5);

    {
        auto& ffm = info.features[4];
        CHECK(ffm.info == &complex_feature::info);
        CHECK(ffm.payload == &pl1);
        CHECK(ffm.bid == 0);
        CHECK(ffm.priority == -3);
    }

    // sanity check that it's kept as is
    {
        auto& ffm = info.features[0];
        CHECK(ffm.info == &simple_feature);
        CHECK(ffm.payload == &pl0);
        CHECK(ffm.bid == 5);
        CHECK(ffm.priority == 6);
    }
}