// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/msg/declare_msg.hpp>

#include <doctest/util/lifetime_counter.hpp>

#include <string>
#include <vector>

struct lc_arg : public doctest::util::lifetime_counter<lc_arg> {
    lc_arg(int i) : val(i) {}
    int val;
    std::vector<int> other_vals;
};

class test_obj;

DYNAMIX_DECLARE_SIMPLE_MSG(get_int, int(test_obj&));
DYNAMIX_DECLARE_SIMPLE_MSG(get_ptr, const void*(const test_obj&));
DYNAMIX_DECLARE_SIMPLE_MSG(simple_mc, int(const test_obj&, int&));
// the allocator arg below is to ensure that commas in type names are supported in simple msg declarations
DYNAMIX_DECLARE_SIMPLE_MSG(simple_uni_vec, void(test_obj&, std::vector<int, std::allocator<int>>));
DYNAMIX_DECLARE_SIMPLE_MSG(simple_multi_arg, void(test_obj&, lc_arg));

DYNAMIX_DECLARE_MSG(overload_i_msg, overloaded, void, (test_obj&, int));
DYNAMIX_DECLARE_MSG(overload_ci_msg, overloaded, int, (const test_obj&, int));
DYNAMIX_DECLARE_MSG(overload_dbl_msg, overloaded, int, (const test_obj&, double&));
DYNAMIX_DECLARE_MSG(uni_vec_msg, uni_vec, void, (test_obj&, std::vector<int>));
DYNAMIX_DECLARE_MSG(multi_arg_msg, multi_arg, void, (test_obj&, lc_arg));
DYNAMIX_DECLARE_MSG(inherited_msg, inherited_func, int, (const test_obj&, int));
DYNAMIX_DECLARE_MSG(fill_vec_msg, fill_vec, void, (const test_obj&, std::vector<int>&));

#include <dynamix/declare_domain.hpp>

#include <dynamix/object.hpp>

DYNAMIX_DECLARE_DOMAIN(test);

#include <dynamix/declare_mixin.hpp>

DYNAMIX_DECLARE_MIXIN(struct common);
DYNAMIX_DECLARE_MIXIN(struct get_ptr_clash);
DYNAMIX_DECLARE_MIXIN(struct over);
DYNAMIX_DECLARE_MIXIN(struct multicaster);
DYNAMIX_DECLARE_MIXIN(class next_impl);

class test_obj : public dynamix::object {
public:
    test_obj() : dynamix::object(dynamix::g::get_domain<test>()) {}
};

#include <dynamix/exception.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>

TEST_CASE("empty obj") {
    test_obj empty;
    {
        auto& t = empty.get_type();
        CHECK(t.implements<get_ptr>());
        CHECK_FALSE(t.implements_strong<get_ptr>());
        CHECK_FALSE(t.implements<get_int>());
        CHECK_FALSE(t.implements<uni_vec_msg>());
        CHECK(t.implements<overload_ci_msg>());
        CHECK_FALSE(t.implements_strong<overload_ci_msg>());
        CHECK_FALSE(t.implements<overload_i_msg>());
    }

    CHECK(get_ptr::call(empty) == &empty);

    CHECK_THROWS_WITH_AS(get_int::call(empty), "dynamix message", dynamix::bad_feature_access);
    CHECK_THROWS_WITH_AS(overloaded(empty, 5), "dynamix message", dynamix::bad_feature_access);

    CHECK(overload_ci_msg::call(empty, 6) == 6);
    const test_obj& cempty = empty;
    CHECK(overloaded(cempty, 5) == 5);
}

TEST_CASE("clash") {
    test_obj obj;
    CHECK_THROWS_WITH_AS(mutate(obj, dynamix::add<common>(), dynamix::add<get_ptr_clash>()),
        "msgt: feature clash in {'common', 'get_ptr_clash'} on 'get_ptr' between 'get_ptr_clash' and 'common'",
        dynamix::mutation_error);
}

TEST_CASE("uni") {
    using namespace dynamix::mutate_ops;

    test_obj obj;
    mutate(obj, add<common>());

    {
        auto& t = obj.get_type();
        CHECK(t.implements_strong<get_ptr>());
        CHECK(t.implements_strong<get_int>());
    }

    CHECK(get_int::call(obj) == 1);
    CHECK(get_int::call(obj) == 2);

    CHECK_FALSE(get_ptr::call(obj));

    {
        // check that value is succesfully moved by call
        std::vector<int> ints = {1, 2, 3, 4, 5};
        auto ints_data = ints.data();
        simple_uni_vec::call(obj, std::move(ints));
        CHECK(get_ptr::call(obj) == ints_data);
    }

    {
        // check that value is succesfully moved by message func
        std::vector<int> ints = {1, 2, 3, 4, 5};
        auto ints_data = ints.data();
        uni_vec(obj, std::move(ints));
        CHECK(get_ptr::call(obj) == ints_data);
    }

    {
        CHECK(overload_ci_msg::call(obj, 3) == 4);
        const auto& cobj = obj;
        CHECK(overloaded(cobj, 2) == 3); // default impl
    }

    mutate(obj, add<over>());
    CHECK(get_int::call(obj) == 100); // overriden
    CHECK(get_int::call(obj) == 99);

    CHECK(inherited_func(obj, 1) == 100);

    overloaded(obj, 5);
    CHECK(inherited_func(obj, 1) == 6);

    {
        CHECK(overload_ci_msg::call(obj, 3) == 2);
        const auto& cobj = obj;
        CHECK(overloaded(cobj, 5) == 0);
    }
}

struct multicaster {
    int val;
    multicaster(int i) : val(i) {}
};

TEST_CASE("multi") {
    using namespace dynamix::mutate_ops;
    test_obj obj;
    mutate(obj, add<multicaster>(5));
    CHECK(obj.get<multicaster>()->val == 5);
    {
        int sum = 0;
        CHECK(simple_mc::call(obj, sum) == 5);
        CHECK(sum == 5);
    }

    {
        lc_arg::lifetime_stats stats;
        simple_multi_arg::call(obj, lc_arg(42));
        int sum = 0;
        CHECK(simple_mc::call(obj, sum) == 42);
        CHECK(sum == 42);
        CHECK(stats.copies == 0);
    }

    obj.get<multicaster>()->val = 5;
    mutate(obj, add<common>());
    get_int::call(obj);
    {
        int sum = 0;
        CHECK(simple_mc::call(obj, sum) == 1);
        CHECK(sum == 6);
    }

    {
        lc_arg::lifetime_stats stats;
        simple_multi_arg::call(obj, lc_arg(13));
        CHECK(obj.get<multicaster>()->val == 13);
        int sum = 0;
        get_int::call(obj);
        CHECK(simple_mc::call(obj, sum) == 14);
        CHECK(sum == 27);
        CHECK(stats.copies == 1);
    }

    mutate(obj, add<over>());
    {
        int sum = 0;
        CHECK(simple_mc::call(obj, sum) == 14);
        CHECK(sum == 27);
    }
}

TEST_CASE("next impl") {
    using namespace dynamix::mutate_ops;
    {
        test_obj obj;
        mutate(obj, add<next_impl>());
        CHECK(get_int::call(obj) == -1);
        CHECK_THROWS_WITH_AS(inherited_func(obj, 1), "next impl", dynamix::bad_feature_access);
    }
    {
        test_obj obj;
        mutate(obj, add<over>(), add<next_impl>());
        CHECK(get_int::call(obj) == 99);
        CHECK(inherited_func(obj, 0) == 100);
    }
    {
        test_obj obj;
        mutate(obj, add<next_impl>());
        int sum = 0;
        CHECK_THROWS_WITH_AS(simple_mc::call(obj, sum), "next bidder set", dynamix::bad_feature_access);
    }
    {
        test_obj obj;
        mutate(obj
            , add<multicaster>(95)
            , add<over>()
            , add<next_impl>()
            , add<common>()
        );
        CHECK(get_int::call(obj) == 1);

        int sum = 1;
        CHECK(simple_mc::call(obj, sum) == 5);
        CHECK(sum == 6);

        sum = 0;
        CHECK(simple_mc::call(obj, sum) == 1);
        CHECK(sum == 96);
    }
}

#include <dynamix/msg/func_traits.hpp>

DYNAMIX_MAKE_FUNC_TRAITS(overloaded);
DYNAMIX_MAKE_FUNC_TRAITS(uni_vec);
DYNAMIX_MAKE_FUNC_TRAITS(multi_arg);
DYNAMIX_MAKE_FUNC_TRAITS(inherited_func);
DYNAMIX_MAKE_FUNC_TRAITS(fill_vec);

#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/next_impl.hpp>

struct common {
    int val = 0;
    std::vector<int> ints;

    void set_ints(std::vector<int>&& vec) {
        ints = std::move(vec);
    }
    void uni_vec(std::vector<int> vec) {
        set_ints(std::move(vec));
    }
};

DYNAMIX_DEFINE_MIXIN(test, common)
    .implements_by<get_int>([](common* cmn) { return ++cmn->val; }) // count
    .implements_by<get_ptr>([](const common* cmn) -> const void* { return cmn->ints.data(); })
    .implements_by<simple_uni_vec>([](common* cmn, std::vector<int> ints) {
        cmn->set_ints(std::move(ints));
    })
    .implements<uni_vec_msg>()
    .implements_by<simple_mc>([](const common* cmn, int& sum) {
        sum += cmn->val;
        return cmn->val;
    })
    .implements_by<simple_multi_arg>([](common* cmn, lc_arg arg) {
        cmn->val = arg.val;
    })
;

struct get_ptr_clash {};
DYNAMIX_DEFINE_MIXIN(test, get_ptr_clash).implements_by<get_ptr>([](const get_ptr_clash*) -> const void* { return nullptr; });

struct parent {
    int val = 101;
    int inherited_func(int i) const {
        return val + i;
    }
};

struct over : public parent {
    void overloaded(int i) { val = i; }
    int overloaded(int i) const { return val - i; }
    void uni_vec(std::vector<int> vec) {
        val = int(vec.size());
    }
};

DYNAMIX_DEFINE_MIXIN(test, over)
    .implements_by<get_int>([](over* o) { return --o->val; }) // count
    .implements<overload_i_msg>()
    .implements<overload_ci_msg>()
    .implements<inherited_msg>()
    .implements<uni_vec_msg>(1_bid)
    .implements_by<simple_mc>([](const over*, int& sum) {
        sum += 10;
        return 10;
    }, -1_bid)
    ;

DYNAMIX_DEFINE_MIXIN(test, multicaster)
    .implements_by<simple_mc>([](const multicaster* mc, int& sum) {
        sum += mc->val;
        return mc->val;
    })
    .implements_by<simple_multi_arg>([](multicaster* mc, lc_arg arg) {
        mc->val = arg.val;
    })
;

class next_impl {
public:
    int double_get_int() {
        if (!DYNAMIX_HAS_NEXT_IMPL_MSG(get_int)) {
            return -1;
        }
        DYNAMIX_CALL_NEXT_IMPL_MSG(get_int);
        return DYNAMIX_CALL_NEXT_IMPL_MSG(get_int);
    }
    int inherited_func(int i) const {
        return DYNAMIX_CALL_NEXT_IMPL_MSG(inherited_msg, i + 1);
    }

    int mc(int& sum) const {
        if (sum == 0) {
            return DYNAMIX_CALL_NEXT_BIDDER_SET(simple_mc, sum);
        }
        sum += 5;
        return 5;
    }
};

DYNAMIX_DEFINE_MIXIN(test, next_impl)
    .implements<inherited_msg>()
    .implements_by<get_int>([](next_impl* ni) { return ni->double_get_int(); })
    .implements_by<simple_mc>([](const next_impl* ni, int& sum) {
        return ni->mc(sum);
    }, 5_bid)
;

#include <dynamix/msg/define_msg.hpp>

DYNAMIX_DEFINE_SIMPLE_MSG(get_int, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(get_ptr, unicast, false, [](const test_obj& obj) -> const void* { return &obj; });
DYNAMIX_DEFINE_SIMPLE_MSG(simple_mc, multicast);
DYNAMIX_DEFINE_SIMPLE_MSG(simple_uni_vec, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(simple_multi_arg, multicast);

int get_num_mixins_plus(const test_obj& obj, int i) {
    return int(obj.get_type().num_mixins()) + i;
}

DYNAMIX_DEFINE_MSG(overload_i_msg, unicast, overloaded, void, (test_obj&, int));
DYNAMIX_DEFINE_MSG_EX(overload_ci_msg, unicast, false, get_num_mixins_plus, overloaded, int, (const test_obj&, int));
DYNAMIX_DEFINE_MSG(overload_dbl_msg, multicast, overloaded, int, (const test_obj&, double&));
DYNAMIX_DEFINE_MSG_EX(uni_vec_msg, unicast, false, nullptr, uni_vec, void, (test_obj&, std::vector<int>));
DYNAMIX_DEFINE_MSG(multi_arg_msg, multicast, multi_arg, void, (test_obj&, lc_arg));
DYNAMIX_DEFINE_MSG(inherited_msg, unicast, inherited_func, int, (const test_obj&, int));
DYNAMIX_DEFINE_MSG(fill_vec_msg, multicast, fill_vec, void, (const test_obj&, std::vector<int>&));

#include <dynamix/define_domain.hpp>

DYNAMIX_DEFINE_DOMAIN(test, "msgt");
