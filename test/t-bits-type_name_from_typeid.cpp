// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/bits/type_name_from_typeid.hpp>

#include <doctest/doctest.h>

struct sfoo {};
class cbar {};

namespace test {
struct a_struct {};
namespace inner {
class some_class {};
}
enum myenum { a, b, c };
}

TEST_CASE("type_name_from_typeid") {
    using dynamix::util::get_type_name_from_typeid;

    CHECK(get_type_name_from_typeid<sfoo>() == "sfoo");
    CHECK(get_type_name_from_typeid<cbar>() == "cbar");
    CHECK(get_type_name_from_typeid<unsigned int>() == "unsigned int");
    CHECK(get_type_name_from_typeid<test::a_struct>() == "test::a_struct");
    CHECK(get_type_name_from_typeid<test::inner::some_class>() == "test::inner::some_class");
    CHECK(get_type_name_from_typeid<test::myenum>() == "test::myenum");
}
