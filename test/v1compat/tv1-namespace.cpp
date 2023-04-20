// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("namespace");

using namespace dynamix::v1compat;

namespace mixins1 {
DYNAMIX_V1_DECLARE_MIXIN(a);
DYNAMIX_V1_DECLARE_MIXIN(b);
}

namespace mixins2 {
DYNAMIX_V1_DECLARE_MIXIN(a);
DYNAMIX_V1_DECLARE_MIXIN(b);
}

TEST_CASE("mixins") {
    object o;
    mutate(o)
        .add<mixins1::a>()
        .add<mixins1::b>()
        .add<mixins2::a>()
        .add<mixins2::b>();

    auto n1a = o.get<mixins1::a>();
    CHECK(n1a == o.get("mixins1::a"));
    auto n2a = o.get<mixins2::a>();
    CHECK(static_cast<void*>(n1a) != static_cast<void*>(n2a));
    CHECK(n2a == o.get("mixins2::a"));
}

namespace mixins1 {
class a {};
class b {};
DYNAMIX_V1_DEFINE_MIXIN(a, dynamix::v1compat::none);
DYNAMIX_V1_DEFINE_MIXIN(b, dynamix::v1compat::none);
}

namespace mixins2 {
class a {};
class b {};

DYNAMIX_V1_DEFINE_MIXIN(a, dynamix::v1compat::none);
DYNAMIX_V1_DEFINE_MIXIN(b, dynamix::v1compat::none);
}
