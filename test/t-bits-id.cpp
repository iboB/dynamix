// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/feature_id.hpp>
#include <dynamix/mixin_id.hpp>

#include <doctest/doctest.h>

static_assert(dynamix::invalid_feature_id.i == dynamix::invalid_mixin_id.i);

template <typename Id>
void id_test() {
    Id i1{10};
    CHECK(i1.i == 10);
    Id i2{42};
    CHECK(i1 != i2);
    i2 = i1;
    CHECK(i2.i == 10);
    CHECK(i2 == i1);
}

TEST_CASE("ids") {
    id_test<dynamix::mixin_id>();
    id_test<dynamix::feature_id>();
}

