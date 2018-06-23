// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include "doctest/doctest.h"

TEST_SUITE("combinators");

DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
DYNAMIX_DECLARE_MIXIN(c);

DYNAMIX_CONST_MULTICAST_MESSAGE_0(bool, check1); // all true
DYNAMIX_CONST_MULTICAST_MESSAGE_0(bool, check2); // some true
DYNAMIX_CONST_MULTICAST_MESSAGE_0(bool, check3); // all false
DYNAMIX_CONST_MULTICAST_MESSAGE_0(int, ival);
DYNAMIX_CONST_MULTICAST_MESSAGE_0(double, dval);

using namespace dynamix;
using namespace dynamix::combinators;

using doctest::Approx;

// some custom combinators
template <int N>
class count_bigger_than
{
public:
    template <typename MessageReturnType>
    class combinator
    {
    public:
        typedef int result_type;
        typedef MessageReturnType type;

        combinator()
            : _result(0)
        {}

        int result() const
        {
            return _result;
        }

        bool add_result(const type& t)
        {
            _result += int(t > _value);
            return true;
        }

    private:
        static const int _value = N;
        int _result;
    };
};

class count_smaller_than
{
public:
    count_smaller_than()
        : _value(0)
        , _count(0)
    {}

    void set_compare_value(double d)
    {
        _value = d;
        _count = 0;
    }

    int count() const
    {
        return _count;
    }

    size_t num_results() const
    {
        return _num_results;
    }

    void set_num_results(size_t num_results)
    {
        _num_results = num_results;
    }

    bool add_result(double d)
    {
        _count += int(d < _value);
        return true;
    }

private:
    double _value;
    int _count;
    size_t _num_results;
};

TEST_CASE("combinators")
{
    object o;
    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>();

    /////////////////////////////////////////
    // ================ and =================
    boolean_and<> b_and;

    CHECK(check1<boolean_and>(o));
    check1(o, b_and);
    CHECK(b_and.result());
    b_and.reset();

    CHECK(!check2<boolean_and>(o));
    check2(o, b_and);
    CHECK(!b_and.result());
    b_and.reset();

    CHECK(!check3<boolean_and>(o));
    check3(o, b_and);
    CHECK(!b_and.result());
    b_and.reset();

    /////////////////////////////////////////
    // ================ or =================
    boolean_or<> b_or;

    CHECK(check1<boolean_or>(o));
    check1(o, b_or);
    CHECK(b_or.result());
    b_or.reset();

    CHECK(check2<boolean_or>(o));
    check2(o, b_or);
    CHECK(b_or.result());
    b_or.reset();

    CHECK(!check3<boolean_or>(o));
    check3(o, b_or);
    CHECK(!b_or.result());
    b_or.reset();

    /////////////////////////////////////////
    // ================ sum =================
    CHECK(ival<sum>(o) == 111);
    CHECK(Approx(dval<sum>(o)) == 0.111);

    /////////////////////////////////////////
    // ================ mean =================
    CHECK(ival<mean>(o) == 37);
    CHECK(Approx(dval<mean>(o)) == 0.037);

    /////////////////////////////////////////
    // ============= custom ===============
    CHECK(ival<count_bigger_than<0>::combinator>(o) == 3);
    CHECK(ival<count_bigger_than<5>::combinator>(o) == 2);
    CHECK(ival<count_bigger_than<50>::combinator>(o) == 1);
    CHECK(ival<count_bigger_than<500>::combinator>(o) == 0);

    count_smaller_than count_smaller;
    dval(o, count_smaller);
    CHECK(count_smaller.count() == 0);
    CHECK(count_smaller.num_results() == 3);

    count_smaller.set_compare_value(0.05);
    dval(o, count_smaller);
    CHECK(count_smaller.count() == 2);

    count_smaller.set_compare_value(1);
    dval(o, count_smaller);
    CHECK(count_smaller.count() == 3);
    CHECK(count_smaller.num_results() == 3);
};


#define all_msg check1_msg & check2_msg & check3_msg & ival_msg & dval_msg

class a
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "a"; }
#endif
    bool check1() const { return true; }
    bool check2() const { return false; }
    bool check3() const { return false; }
    int ival() const { return 1; }
    double dval() const { return 0.1; }
};

DYNAMIX_DEFINE_MIXIN(a, all_msg);

class b
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "b"; }
#endif
    bool check1() const { return true; }
    bool check2() const { return true; }
    bool check3() const { return false; }
    int ival() const { return 10; }
    double dval() const { return 0.01; }
};

DYNAMIX_DEFINE_MIXIN(b, all_msg);

class c
{
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "c"; }
#endif
    bool check1() const { return true; }
    bool check2() const { return false; }
    bool check3() const { return false; }
    int ival() const { return 100; }
    double dval() const { return 0.001; }
};

DYNAMIX_DEFINE_MIXIN(c, all_msg);

DYNAMIX_DEFINE_MESSAGE(check1);
DYNAMIX_DEFINE_MESSAGE(check2);
DYNAMIX_DEFINE_MESSAGE(check3);
DYNAMIX_DEFINE_MESSAGE(ival);
DYNAMIX_DEFINE_MESSAGE(dval);

