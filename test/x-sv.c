// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "x-sv.h"
#include <dnmx/bits/sv.h>
#include "s-unity.h"

static void sv(void) {
    dnmx_sv e = {0};
    T_NULL(e.begin);
    T_NULL(e.end);
    T_TRUE(dnmx_sv_is_null(e));
    T_TRUE(dnmx_sv_is_empty(e));
    T_TRUE(dnmx_sv_len(e) == 0);
    T_TRUE(dnmx_sv_cmp(e, e) == 0);
    T_TRUE(dnmx_sv_starts_with(e, ""));
    T_FALSE(dnmx_sv_starts_with(e, "x"));
    T_NULL(dnmx_sv_find_first(e, 'x'));
    T_NULL(dnmx_sv_find_last(e, 'x'));

    dnmx_sv e2 = dnmx_make_sv_str("");
    T_NOT_NULL(e2.begin);
    T_NOT_NULL(e2.end);
    T_FALSE(dnmx_sv_is_null(e2));
    T_TRUE(dnmx_sv_is_empty(e2));
    T_TRUE(dnmx_sv_len(e2) == 0);
    T_TRUE(dnmx_sv_cmp(e2, e2) == 0);
    T_TRUE(dnmx_sv_cmp(e, e2) == 0);
    T_TRUE(dnmx_sv_cmp(e2, e) == 0);
    T_TRUE(dnmx_sv_starts_with(e2, ""));
    T_FALSE(dnmx_sv_starts_with(e2, "x"));
    T_NULL(dnmx_sv_find_first(e2, 'x'));
    T_NULL(dnmx_sv_find_last(e2, 'x'));

    {
        dnmx_sv abc = dnmx_make_sv_str("abc");
        T_NOT_NULL(abc.begin);
        T_NOT_NULL(abc.end);
        T_FALSE(dnmx_sv_is_null(abc));
        T_FALSE(dnmx_sv_is_empty(abc));
        T_TRUE(dnmx_sv_len(abc) == 3);
        T_TRUE(dnmx_sv_cmp(abc, abc) == 0);
        T_TRUE(dnmx_sv_cmp(e, abc) == -1);
        T_TRUE(dnmx_sv_cmp(abc, e) == 1);
        T_TRUE(dnmx_sv_cmp(e2, abc) == -1);
        T_TRUE(dnmx_sv_cmp(abc, e2) == 1);
        T_TRUE(dnmx_sv_starts_with(abc, "a"));
        T_TRUE(dnmx_sv_starts_with(abc, "ab"));
        T_TRUE(dnmx_sv_starts_with(abc, "abc"));
        T_FALSE(dnmx_sv_starts_with(abc, "abcd"));
        T_FALSE(dnmx_sv_starts_with(abc, "x"));
        T_NULL(dnmx_sv_find_first(abc, 'x'));
        T_NULL(dnmx_sv_find_last(abc, 'x'));
        const char* p = dnmx_sv_find_first(abc, 'b');
        T_NOT_NULL(p);
        const char* p2 = dnmx_sv_find_last(abc, 'b');
        T_EQ_PTR(p, p2);
        dnmx_sv bc = dnmx_make_sv_be(p, abc.end);
        T_SV_EXPECT("bc", bc);
    }

    {
        dnmx_sv foof = dnmx_make_sv_str("foof");
        T_NULL(dnmx_sv_find_first(foof, 'x'));
        const char* p = dnmx_sv_find_first(foof, 'o');
        T_NOT_NULL(p);
        dnmx_sv oof = dnmx_make_sv_be(p, foof.end);
        T_SV_EXPECT("oof", oof);
        const char* p2 = dnmx_sv_find_last(foof, 'o');
        T_EQ_PTR(p + 1, p2);
        dnmx_sv fo = dnmx_make_sv_be(foof.begin, p2);
        T_SV_EXPECT("fo", fo);
    }

    {
        dnmx_sv lit = dnmx_make_sv_lit("asdf");
        T_EQ(4, dnmx_sv_len(lit));
        T_SV_EXPECT("asdf", lit);
    }
}

bool cxx_to_c(dnmx_sv* a, dnmx_sv* b) {
    bool ret = dnmx_sv_cmp(*a, *b) == 0;
    --a->end;
    ++a->begin;
    *b = dnmx_make_sv_lit("hello from c");
    return ret;
}

static void cross(void) {
    dnmx_sv a = dnmx_make_sv_lit("c here");
    dnmx_sv b = a;
    c_to_cxx(&a, &b);
    T_SV_EXPECT("c++ here", a);
    T_SV_EXPECT("and again here", b);
}

void run_unity_tests() {
    RUN_TEST(sv);
    RUN_TEST(cross);
}
