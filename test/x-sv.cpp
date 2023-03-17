// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "x-sv.h"
#include <dnmx/bits/sv.h>
#include <doctest/doctest.h>

TEST_CASE("dnmx_sv") {
    dnmx_sv a = {};
    CHECK_FALSE(a.begin);
    CHECK_FALSE(a.end);
    CHECK(a.empty());

    std::string_view sv = a.to_std();
    CHECK(sv.empty());

    CHECK(a == "");
    CHECK(a == sv);
    CHECK(a != "x");

    a = dnmx_sv::from_std("asdf");
    CHECK(a == "asdf");
    CHECK(a != "a");
    CHECK(sv != a);

    sv = "asdf";
    CHECK(sv == a);

    dnmx_sv b = dnmx_sv::from_std("asdf");
    CHECK(a == b);

    b = dnmx_sv::from_std("azdf");
    CHECK(a < b);
    CHECK(a != b);
}

extern "C" {
bool c_to_cxx(dnmx_sv* a, dnmx_sv* b) {
    auto ret = *a == *b;
    *a = dnmx_sv::from_std("c++ here");
    *b = dnmx_sv::from_std("and again here");
    return ret;
}
}

TEST_CASE("cross") {
    dnmx_sv a = dnmx_sv::from_std("it's c++ calling");
    dnmx_sv b = a;
    CHECK(cxx_to_c(&a, &b));
    CHECK(a == "t's c++ callin");
    CHECK(b == "hello from c");
}
