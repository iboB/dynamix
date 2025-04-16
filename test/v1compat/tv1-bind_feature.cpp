// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/v1compat/combinators.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("v1 bind feature");

using namespace dynamix::v1compat;

DYNAMIX_V1_MESSAGE_1(void, set_a, int, val);
DYNAMIX_V1_MESSAGE_1(void, set_b, int, val);
DYNAMIX_V1_CONST_MULTICAST_MESSAGE_0(int, multi);
DYNAMIX_V1_CREATE_COMBINATOR_CALL_0(int, multi);

struct a {
    int ia;
};

void a_set_a(void* vself, int val) {
    auto self = reinterpret_cast<a*>(vself);
    self->ia = val;
}

// v2!: consntess is respected on bind
int a_multi(const void* vself) {
    auto self = reinterpret_cast<const a*>(vself);
    return self->ia;
}

DYNAMIX_V1_DEFINE_MIXIN(a, bind(set_a_msg, a_set_a) & bind(multi_msg, a_multi));

struct a2 {
    int ia2;
};

void a2_set_a(void* vself, int val) {
    auto self = reinterpret_cast<a2*>(vself);
    self->ia2 = val + 1;
}

// v2!: consntess is respected on bind
int a2_multi(const void* vself) {
    auto self = reinterpret_cast<const a2*>(vself);
    return self->ia2 + 1;
}

DYNAMIX_V1_DEFINE_MIXIN(a2, bind(set_a_msg, a2_set_a) & bind(multi_msg, a2_multi));

struct b {
    int ib;
    int multi() const {
        return ib;
    }
};

void b_set_b(void* vself, int val) {
    auto self = reinterpret_cast<b*>(vself);
    self->ib = val;
}

DYNAMIX_V1_DEFINE_MIXIN(b, multi_msg & bind(set_b_msg, b_set_b));

TEST_CASE("bind msg") {
    object ab;
    mutate(ab).add<a>().add<b>();
    set_a(ab, 1);
    CHECK(ab.get<a>()->ia == 1);
    set_b(ab, 2);
    CHECK(ab.get<b>()->ib == 2);
    CHECK(multi<combinators::sum>(ab) == 3);

    object a2b;
    mutate(a2b).add<a2>().add<b>();
    set_a(a2b, 1);
    CHECK(a2b.get<a2>()->ia2 == 2);
    set_b(a2b, 2);
    CHECK(multi<combinators::sum>(a2b) == 5);
}

DYNAMIX_V1_DEFINE_MESSAGE(set_a);
DYNAMIX_V1_DEFINE_MESSAGE(set_b);
DYNAMIX_V1_DEFINE_MESSAGE(multi);
