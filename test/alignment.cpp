// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_SUITE("align");

using namespace dynamix;

TEST_CASE("sub_ptr_align")
{
    size_t ptr_size = sizeof(uintptr_t);
    for(size_t i=1; i<=ptr_size; ++i)
    {
        CHECK(2 * ptr_size ==
            global_allocator::calculate_mem_size_for_mixin(i, 1));
    }

    for(size_t i=2; i<=ptr_size; i+=2)
    {
        CHECK(2 * ptr_size ==
            global_allocator::calculate_mem_size_for_mixin(i, 2));
    }
}

DYNAMIX_DECLARE_MIXIN(align_default);
DYNAMIX_DECLARE_MIXIN(align_8);
DYNAMIX_DECLARE_MIXIN(align_16);
DYNAMIX_DECLARE_MIXIN(align_32);

DYNAMIX_MULTICAST_MESSAGE_0(void, check_alignment);

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

class align_default
{
public:
    void check_alignment()
    {
        CHECK(intptr_t(this) % std::alignment_of<align_default>::value == 0);
    }
};

#if defined _MSC_VER
#   define ALIGN(n) __declspec(align(n))
#else
#   define ALIGN(n) __attribute__((aligned(n)))
#endif


class ALIGN(16) align_8
{
public:
    void check_alignment()
    {
        CHECK(intptr_t(this) % 8 == 0);
    }
};

class ALIGN(16) align_16
{
public:
    void check_alignment()
    {
        CHECK(intptr_t(this) % 16 == 0);
    }
};

class ALIGN(32) align_32
{
public:
    void check_alignment()
    {
        CHECK(intptr_t(this) % 32 == 0);
    }
};

DYNAMIX_DEFINE_MIXIN(align_default, check_alignment_msg);
DYNAMIX_DEFINE_MIXIN(align_8, check_alignment_msg);
DYNAMIX_DEFINE_MIXIN(align_16, check_alignment_msg);
DYNAMIX_DEFINE_MIXIN(align_32, check_alignment_msg);

DYNAMIX_DEFINE_MESSAGE(check_alignment);
