// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/feature_info_data.hpp>

#include <doctest/util/lifetime_counter.hpp>

using namespace dynamix::util;

TEST_CASE("feature_info_data builder") {
    feature_info_data data;

    feature_info_data_builder b(data, "foo");
    const auto& info = data.info;

    CHECK(info.name.to_std() == "foo");

    b.store_name("bar");
    CHECK(info.name.to_std() == "bar");
    CHECK(data.stored_name == "bar");
    CHECK((const void*)data.stored_name.c_str() == info.name.begin);

    b.name("xxx");
    CHECK((const void*)data.stored_name.c_str() != info.name.begin);
    CHECK(info.name.to_std() == "xxx");

    CHECK_FALSE(info.default_payload);
    int pl0 = 4;
    b.default_payload_by(pl0);
    CHECK(info.default_payload == &pl0);
    CHECK(info.default_payload == data.default_payload_storage.get());

    b.default_payload_with(43);
    CHECK(*static_cast<int*>(info.default_payload) == 43);
    CHECK(info.default_payload == data.default_payload_storage.get());

    b.default_payload_by(pl0);
    CHECK(info.default_payload == &pl0);
    CHECK(info.default_payload == data.default_payload_storage.get());
}
