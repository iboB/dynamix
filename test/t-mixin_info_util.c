// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/mixin_info_util.h>
#include <splat/warnings.h>

#include "s-unity.h"

void setUp(void) {}
void tearDown(void) {}

void basics(void) {
    // just test select values
    dnmx_mixin_info info = dnmx_make_mixin_info();
    CHECK(info.id.i == dnmx_invalid_id);
    T_NULL(info.dom);
    T_NULL(info.equals);
    T_NULL(info.allocator);
    CHECK(info.obj_buf_size == sizeof(void*));
    CHECK(info.mixin_class == 0);
    CHECK(dnmx_sv_is_empty(info.name));
}

#define SIZE 16
void fill(char* buf) {
    for (char i = 0; i < SIZE; ++i) {
        buf[(int)i] = i + 1;
    }
}
bool is_filled(const char* buf) {
    for (int i = 0; i < SIZE; ++i) {
        if (buf[i] != i + 1) return false;
    }
    return true;
}
void clear(char* buf) {
    memset(buf, 0, SIZE);
}
bool is_cleared(const char* buf) {
    for (int i = 0; i < SIZE; ++i) {
        if (buf[i] != 0) return false;
    }
    return true;
}
bool is_fe(const char* buf) {
    for (int i = 0; i < SIZE; ++i) {
        PRAGMA_WARNING_PUSH
        DISABLE_MSVC_WARNING(4310) // cast truncates constant value
        if (buf[i] != (char)0xfe) return false;
        PRAGMA_WARNING_POP
    }
    return true;
}

void funcs(void) {
    char buf_a[SIZE + 5] = {0};
    CHECK(is_cleared(buf_a)); // sanity

    buf_a[SIZE] = 42;
    buf_a[SIZE + 1] = 66;

    char buf_b[SIZE + 5] = {0};
    buf_b[SIZE] = 120;
    buf_b[SIZE + 1] = 1;

    dnmx_mixin_info info = dnmx_make_mixin_info();
    info.size = SIZE;
    info.alignment = 1;

    fill(buf_a);
    CHECK(is_filled(buf_a)); // sanity
    T_SUCCESS(dnmx_mixin_common_init_func(&info, buf_a));
    CHECK(is_cleared(buf_a));

    fill(buf_a);
    T_SUCCESS(dnmx_mixin_common_copy_func(&info, buf_b, buf_a));
    CHECK(is_filled(buf_b));
    clear(buf_b);
    CHECK(is_cleared(buf_b)); // sanity

    dnmx_mixin_common_move_func(&info, buf_b, buf_a);
    CHECK(is_filled(buf_b));
    CHECK(is_cleared(buf_a));

    dnmx_mixin_common_move_func_copy(&info, buf_a, buf_b);
    CHECK(is_filled(buf_b));
    CHECK(is_filled(buf_a));

    CHECK(dnmx_mixin_common_cmp_func(&info, buf_a, buf_b) == 0);
    CHECK(dnmx_mixin_common_eq_func(&info, buf_a, buf_b));
    buf_a[3] = 5;
    CHECK(dnmx_mixin_common_cmp_func(&info, buf_a, buf_b) > 0);
    CHECK_FALSE(dnmx_mixin_common_eq_func(&info, buf_a, buf_b));
    buf_b[3] = 15;
    CHECK(dnmx_mixin_common_cmp_func(&info, buf_a, buf_b) < 0);
    CHECK_FALSE(dnmx_mixin_common_eq_func(&info, buf_a, buf_b));

    buf_a[3] = buf_b[3] = 4;
    T_SUCCESS(dnmx_mixin_noop_init_func(&info, buf_a));
    CHECK(is_filled(buf_a));

    clear(buf_a);
    dnmx_mixin_noop_move_func(&info, buf_a, buf_b);
    dnmx_mixin_noop_copy_func(&info, buf_a, buf_b);
    CHECK(is_cleared(buf_a));
    CHECK(is_filled(buf_b));

    dnmx_mixin_common_destroy_func(&info, buf_a);
    CHECK(is_fe(buf_a));
}

typedef struct byte { uint8_t val; } byte;

typedef struct vec3 {
    alignas(16) double x;
    double y, z;
} vec3;

void common_init(void) {
    dnmx_mixin_info info;
    DNMX_COMMON_INIT_MI(info, byte);

    T_SV_EXPECT("byte", info.name);
    CHECK(info.size == 1);
    CHECK(info.alignment == 1);
    CHECK(info.obj_buf_size == sizeof(void*) + 1);
    CHECK(info.obj_buf_alignment_and_mixin_offset == sizeof(void*));

    CHECK(info.id.i == dnmx_invalid_id);
    T_NULL(info.dom);
    CHECK(info.mixin_class == 0);

    CHECK(info.init == dnmx_mixin_common_init_func);
    CHECK(info.copy_init == dnmx_mixin_common_copy_func);
    CHECK(info.move_init == dnmx_mixin_common_move_func);
    CHECK(info.copy_asgn == dnmx_mixin_common_copy_func);
    CHECK(info.move_asgn == dnmx_mixin_common_move_func);
    T_NULL(info.compare);

    DNMX_COMMON_INIT_MI(info, vec3);

    T_SV_EXPECT("vec3", info.name);
    CHECK(info.size == 32);
    CHECK(info.alignment == 16);
    CHECK(info.obj_buf_size == 48);
    CHECK(info.obj_buf_alignment_and_mixin_offset == 16);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(basics);
    RUN_TEST(funcs);
    RUN_TEST(common_init);
    return UNITY_END();
}
