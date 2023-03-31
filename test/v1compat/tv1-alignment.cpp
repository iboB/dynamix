// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("v1 align");

using namespace dynamix::v1compat;

DYNAMIX_V1_DECLARE_MIXIN(align_default);
DYNAMIX_V1_DECLARE_MIXIN(align_8);
DYNAMIX_V1_DECLARE_MIXIN(align_16);
DYNAMIX_V1_DECLARE_MIXIN(align_32);

DYNAMIX_V1_MULTICAST_MESSAGE_0(void, check_alignment);

TEST_CASE("aligned_mixin")
{
    object o;

    mutate(o)
        .add<align_default>()
        .add<align_8>()
        .add<align_16>()
        .add<align_32>();

    CHECK(intptr_t(o.get<align_8>()) % 8 == 0);
    CHECK(intptr_t(o.get<align_16>()) % 16 == 0);
    CHECK(intptr_t(o.get<align_32>()) % 32 == 0);

    check_alignment(o);
}

TEST_SUITE_END();

class align_default {
public:
    void check_alignment() {
        CHECK(intptr_t(this) % std::alignment_of<align_default>::value == 0);
    }
};

DISABLE_MSVC_WARNING(4324) // disable padding waring for types below

class alignas(8) align_8 {
public:
    void check_alignment() {
        CHECK(intptr_t(this) % 8 == 0);
    }
};

class alignas(16) align_16 {
public:
    void check_alignment() {
        CHECK(intptr_t(this) % 16 == 0);
    }
};

class alignas(32) align_32 {
public:
    void check_alignment() {
        CHECK(intptr_t(this) % 32 == 0);
    }
};

DYNAMIX_V1_DEFINE_MIXIN(align_default, check_alignment_msg);
DYNAMIX_V1_DEFINE_MIXIN(align_8, check_alignment_msg);
DYNAMIX_V1_DEFINE_MIXIN(align_16, check_alignment_msg);
DYNAMIX_V1_DEFINE_MIXIN(align_32, check_alignment_msg);

DYNAMIX_V1_DEFINE_MESSAGE(check_alignment);
