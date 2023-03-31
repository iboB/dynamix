// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/mixin_info_util.hpp>
#include <dynamix/common_mixin_info.hpp>

#include <doctest/util/lifetime_counter.hpp>

#include <splat/warnings.h>

#define long_string "something long... really long... yeah hoho"

struct cnt : public doctest::util::lifetime_counter<cnt> {
    int i = 5;
    std::string str = long_string;
};
using cnt_storage = std::aligned_storage_t<sizeof(cnt), alignof(cnt)>;

cnt& from_storage(cnt_storage& s) {
    return *reinterpret_cast<cnt*>(&s);
}

using namespace dynamix;

TEST_CASE("funcs") {
    cnt::lifetime_stats _s;
    doctest::util::lifetime_counter_sentry _ss(_s);

    // pointer should not be touched by the functions tested here
    mixin_info* info = nullptr;

    {
        cnt::lifetime_stats stats;
        cnt_storage buf;
        util::call_mixin_default_ctor<cnt>(info, &buf);
        CHECK(stats.living == 1);
        CHECK(stats.d_ctr == 1);
        auto& val = from_storage(buf);
        CHECK(val.i == 5);
        CHECK(val.str == long_string);
        util::call_mixin_dtor<cnt>(info, &buf);
        CHECK(stats.living == 0);
        CHECK(stats.total == 1);
    }

    {
        cnt::lifetime_stats stats;
        cnt source;
        source.i = 42;
        source.str = "asdf";
        cnt_storage buf;
        util::call_mixin_copy_ctor<cnt>(info, &buf, &source);
        CHECK(stats.living == 2);
        CHECK(stats.d_ctr == 1);
        CHECK(stats.c_ctr == 1);
        auto& val = from_storage(buf);
        CHECK(val.i == 42);
        CHECK(val.str == "asdf");
        util::call_mixin_dtor<cnt>(info, &buf);
        CHECK(stats.living == 1);
        CHECK(stats.total == 2);
    }

    {
        cnt::lifetime_stats stats;
        cnt source;
        cnt_storage buf;
        util::call_mixin_move_ctor<cnt>(info, &buf, &source);
        CHECK(stats.living == 2);
        CHECK(stats.d_ctr == 1);
        CHECK(stats.m_ctr == 1);
        auto& val = from_storage(buf);
        CHECK(val.i == 5);
        CHECK(val.str == long_string);
        CHECK(source.str.empty());
        util::call_mixin_dtor<cnt>(info, &buf);
        CHECK(stats.living == 1);
        CHECK(stats.total == 2);
    }

    {
        cnt::lifetime_stats stats;
        cnt source;
        source.i = 42;
        source.str = "asdf";
        cnt val;

        util::call_mixin_copy_assignment<cnt>(info, &val, &source);
        CHECK(stats.c_asgn == 1);
        CHECK(val.i == 42);
        CHECK(val.str == "asdf");
    }

    {
        cnt::lifetime_stats stats;
        cnt source;
        source.i = 42;
        source.str = "asdf";
        cnt val;

        util::call_mixin_move_assignment<cnt>(info, &val, &source);
        CHECK(stats.m_asgn == 1);
        CHECK(val.i == 42);
        CHECK(val.str == "asdf");
        CHECK(source.str.empty());
    }

    {
        int a = 5;
        int b = 10;
        CHECK_FALSE(util::call_mixin_equality_operator<int>(info, &a, &b));
        b = 5;
        CHECK(util::call_mixin_equality_operator<int>(info, &a, &b));
    }
}

struct byte { uint8_t val; };

DISABLE_MSVC_WARNING(4324) // disable padding waring for type below
struct alignas(16) vec3 {
    double x, y, z;

    vec3() = delete;
};

TEST_CASE("common_mixin_info") {
    {
        common_mixin_info info;
        CHECK(info.id.i == dnmx_invalid_id);
        CHECK_FALSE(info.dom);
        CHECK(!info.equals);
        CHECK_FALSE(info.allocator);
        CHECK(info.obj_buf_size == sizeof(void*));
        CHECK(info.mixin_class == 0);
        CHECK(dnmx_sv_is_empty(info.name));
    }

    {
        common_mixin_info info;
        util::common_init_mixin_info<void>(info);
        CHECK(info.name.to_std().empty());
        CHECK(info.size == 0);
        CHECK(info.alignment == 0);
        CHECK(info.obj_buf_size == sizeof(void*));
        CHECK(info.obj_buf_alignment_and_mixin_offset == sizeof(void*));
        CHECK((info.init == &dnmx_mixin_noop_init_func));
        CHECK((info.copy_init == &dnmx_mixin_noop_copy_func));
        CHECK((info.move_init == &dnmx_mixin_noop_move_func));
        CHECK((info.move_asgn == &dnmx_mixin_noop_move_func));
        CHECK((info.copy_init == &dnmx_mixin_noop_copy_func));
        CHECK((info.copy_asgn == &dnmx_mixin_noop_copy_func));
        CHECK((info.compare == &dnmx_mixin_noop_cmp_func));
        CHECK(!info.destroy);
    }

    {
        common_mixin_info info;
        DYNAMIX_COMMON_INIT_MI(info, byte);
        CHECK(info.name.to_std() == "byte");
        CHECK(info.size == 1);
        CHECK(info.alignment == 1);
        CHECK(info.obj_buf_size == sizeof(void*) + 1);
        CHECK(info.obj_buf_alignment_and_mixin_offset == sizeof(void*));
        CHECK((info.init == &dnmx_mixin_common_init_func));
        CHECK((info.copy_init == &dnmx_mixin_common_copy_func));
        CHECK((info.move_init == &dnmx_mixin_common_move_func));
        CHECK((info.move_asgn == &dnmx_mixin_common_move_func));
        CHECK((info.copy_init == &dnmx_mixin_common_copy_func));
        CHECK((info.copy_asgn == &dnmx_mixin_common_copy_func));
        CHECK(!info.destroy);
    }

    {
        common_mixin_info info;
        DYNAMIX_COMMON_INIT_MI(info, cnt);
        CHECK(info.name.to_std() == "cnt");
        CHECK((info.init == &util::call_mixin_default_ctor<cnt>));
        CHECK((info.copy_init == &util::call_mixin_copy_ctor<cnt>));
        CHECK((info.move_init == &util::call_mixin_move_ctor<cnt>));
        CHECK((info.copy_asgn == &util::call_mixin_copy_assignment<cnt>));
        CHECK((info.move_asgn == &util::call_mixin_move_assignment<cnt>));
        CHECK((info.destroy == &util::call_mixin_dtor<cnt>));
    }

    {
        common_mixin_info info;
        DYNAMIX_COMMON_INIT_MI(info, vec3);
        CHECK(info.size == 32);
        CHECK(info.alignment == 16);
        CHECK(info.obj_buf_size == 48);
        CHECK(info.obj_buf_alignment_and_mixin_offset == 16);
        CHECK(!info.init);
        CHECK((info.copy_init == &dnmx_mixin_common_copy_func));
        CHECK((info.move_init == &dnmx_mixin_common_move_func));
        CHECK((info.move_asgn == &dnmx_mixin_common_move_func));
        CHECK((info.copy_init == &dnmx_mixin_common_copy_func));
        CHECK((info.copy_asgn == &dnmx_mixin_common_copy_func));
        CHECK(!info.destroy);
    }
}
