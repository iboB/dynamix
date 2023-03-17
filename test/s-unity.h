// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

// include unity
// and add shorthand macros

#include <unity.h>

#define CHECK       TEST_ASSERT
#define CHECK_FALSE TEST_ASSERT_FALSE

#define T_NULL      TEST_ASSERT_NULL
#define T_NOT_NULL  TEST_ASSERT_NOT_NULL
#define T_EQ        TEST_ASSERT_EQUAL
#define T_EQ_PTR    TEST_ASSERT_EQUAL_PTR
#define T_TRUE      TEST_ASSERT_TRUE
#define T_FALSE     TEST_ASSERT_FALSE
#define T_GT        TEST_ASSERT_GREATER_THAN

#define T_SUCCESS(x) T_EQ(0, x)
#define T_FAIL(x) TEST_ASSERT_NOT_EQUAL(0, x)

#define T_SV_EQ(a, b) T_TRUE(dnmx_sv_eq(a, b))
#define T_SV_EXPECT(expected, sv) T_SV_EQ(dnmx_make_sv_str(expected), sv)

#include <splat/warnings.h>
DISABLE_GCC_AND_CLANG_WARNING("-Wmissing-field-initializers")
