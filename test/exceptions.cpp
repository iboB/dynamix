// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#if DYNAMIX_USE_EXCEPTIONS

#include "doctest/doctest.h"

TEST_SUITE("exceptions");

using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(has_u1m1);
DYNAMIX_DECLARE_MIXIN(has_u1);
DYNAMIX_DECLARE_MIXIN(has_m1);
DYNAMIX_DECLARE_MIXIN(has_u2m2);
DYNAMIX_DECLARE_MIXIN(has_u2);
DYNAMIX_DECLARE_MIXIN(has_m2);

DYNAMIX_MULTICAST_MESSAGE_0(int, m1);
DYNAMIX_MULTICAST_MESSAGE_0(int, m2);
DYNAMIX_MESSAGE_0(void, u1);
DYNAMIX_MESSAGE_0(void, u2);
DYNAMIX_MESSAGE_0(void, unused);

TEST_CASE("ex_bad_message_call")
{
    object o;

    mutate(o)
        .add<has_u1m1>()
        .add<has_m1>();

    CHECK_NOTHROW(u1(o));
    CHECK_NOTHROW(m1(o));
    CHECK_NOTHROW(m1<combinators::sum>(o));

    combinators::sum<int> sum;
    CHECK_NOTHROW(m1(o, sum));

    CHECK_THROWS_AS(u2(o), bad_message_call);
    CHECK_THROWS_AS(m2(o), bad_message_call);
    CHECK_THROWS_AS(m2<combinators::sum>(o), bad_message_call);
    CHECK_THROWS_AS(m2(o, sum), bad_message_call);
    CHECK_THROWS_AS(unused(o), bad_message_call);
}

TEST_CASE("ex_bad_mutation_source")
{
    object o1;
    mutate(o1)
        .add<has_u1m1>()
        .add<has_m1>();

    object o2;
    mutate(o2)
        .add<has_u1m1>()
        .add<has_m1>();

    object o3;
    mutate(o3)
        .add<has_u1m1>()
        .add<has_m1>();

    same_type_mutator mut_1_to_2;
    mut_1_to_2
        .remove<has_u1m1>()
        .remove<has_m1>()
        .add<has_u2m2>()
        .add<has_m2>();

    CHECK_NOTHROW(mut_1_to_2.apply_to(o1));
    CHECK_NOTHROW(mut_1_to_2.apply_to(o2));

    // here o1 and o2 are now type u2m2

    same_type_mutator mut_2_to_1;
    mut_2_to_1
        .remove<has_u2m2>()
        .remove<has_m2>()
        .add<has_u1m1>()
        .add<has_m1>();

    CHECK_NOTHROW(mut_2_to_1.apply_to(o1));
    CHECK_NOTHROW(mut_2_to_1.apply_to(o2));

    CHECK_THROWS_AS(mut_2_to_1.apply_to(o3), bad_mutation_source);
}

TEST_CASE("ex_unicast_clash")
{
    object o;

    single_object_mutator safe(o);
    safe
        .add<has_u1>()
        .add<has_u2m2>();

    CHECK_NOTHROW(safe.apply());

    object o2;

    single_object_mutator clash(o2);
    clash
        .add<has_u2>()
        .add<has_u2m2>();

    CHECK_THROWS_AS(clash.apply(), unicast_clash);

    single_object_mutator clash2(o);
    clash2
        .add<has_u2>();

    CHECK_THROWS_AS(clash2.apply(), unicast_clash);

    object_type_template safe_t;
    safe_t
        .add<has_u2>()
        .add<has_u1m1>();

    CHECK_NOTHROW(safe_t.create());

    object_type_template clash_t;
    clash_t
        .add<has_u2>()
        .add<has_u2m2>();

    CHECK_THROWS_AS(clash_t.create(), unicast_clash);
}

TEST_CASE("ex_bad_mutation")
{
    object_type_template tmpl;
    tmpl
        .add<has_u1m1>()
        .add<has_u2m2>();

    CHECK_THROWS_AS(object o(tmpl), bad_mutation);

    tmpl.create();

    CHECK_NOTHROW(object o(tmpl));

    // the type template should be usable
    object o(tmpl);
    CHECK(o.has<has_u1m1>());
    CHECK(o.has<has_u2m2>());
}

class has_u1m1
{
public:
    void u1() {}
    int m1() { return 1; }
};

class has_u1
{
public:
    void u1() {}
};

class has_m1
{
public:
    int m1() { return 2; }
};

class has_u2m2
{
public:
    void u2() {}
    int m2() { return 3; }
};

class has_u2
{
public:
    void u2() {}
};

class has_m2
{
public:
    int m2() { return 4; }
};

DYNAMIX_DEFINE_MIXIN(has_u1m1, u1_msg & m1_msg);
DYNAMIX_DEFINE_MIXIN(has_m1, m1_msg);
DYNAMIX_DEFINE_MIXIN(has_u1, u1_msg);
DYNAMIX_DEFINE_MIXIN(has_u2m2, u2_msg & m2_msg);
DYNAMIX_DEFINE_MIXIN(has_m2, m2_msg);
DYNAMIX_DEFINE_MIXIN(has_u2, u2_msg);

DYNAMIX_DEFINE_MESSAGE(m1);
DYNAMIX_DEFINE_MESSAGE(m2);
DYNAMIX_DEFINE_MESSAGE(u1);
DYNAMIX_DEFINE_MESSAGE(u2);
DYNAMIX_DEFINE_MESSAGE(unused);

#else
int main()
{
    // tests not applicalble for this case
    return 0;
}
#endif
