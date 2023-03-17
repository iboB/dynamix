// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/bits/make_from_tuple.hpp>
#include <doctest/doctest.h>

#include <string>
#include <vector>
#include <string_view>

struct isv {
    int i;
    std::string str;
    std::vector<int> vec;

    isv(int i) : i(i) {}
    isv(std::string_view sv, std::vector<int> vec) : i(13), str(sv), vec(std::move(vec)) {}
};

struct isv_maker {
    union {
        isv data;
    };
    bool constructed = false;

    template <typename Tuple>
    isv_maker(Tuple&& tup) {
        dynamix::impl::make_from_tuple<isv>(&data, std::move(tup));
    }

    ~isv_maker() {
        data.~isv();
    }
};

TEST_CASE("make_form_tuple") {
    {
        isv_maker h(std::make_tuple(5));
        CHECK(h.data.i == 5);
    }
    {
        std::vector<int> ints = {1, 2, 3};
        auto ints_data = ints.data();
        isv_maker h(std::make_tuple("asdf", std::move(ints)));
        CHECK(h.data.i == 13);
        CHECK(h.data.str == "asdf");
        CHECK(h.data.vec == std::vector<int>{1, 2, 3});
        CHECK(h.data.vec.data() == ints_data);
    }
}
