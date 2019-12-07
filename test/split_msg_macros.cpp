// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#define DYNAMIX_NO_MESSAGE_MACROS
#include <dynamix/declare_mixin.hpp>

#include "doctest/doctest.h"

TEST_SUITE("split");

using namespace dynamix;

// some mixins and messages
DYNAMIX_DECLARE_MIXIN(counter);
DYNAMIX_DECLARE_MIXIN(no_messages);
DYNAMIX_DECLARE_MIXIN(type_checker);
//DYNAMIX_DECLARE_MIXIN(overrider);
//DYNAMIX_DECLARE_MIXIN(foo);
//DYNAMIX_DECLARE_MIXIN(bar);

#include <dynamix/declare_message_split.hpp>
#include "split_msg_macros_messages.inl"

class no_messages
{
};

class counter
{
public:
    counter()
        : _count(0)
    {}

    void dummy() {}

    void count_uni() { ++_count; }
    void count_multi();

    int get_count() const { return _count; }

    void multi(int& n)
    {
        ++n;
    }

private:
    int _count;
};

// issue #20 test
struct parent
{
    int inherited()
    {
        return 22;
    }
};

class type_checker : public parent
{
public:
    const void* get_self() const
    {
        return this;
    }

    void multi(int& n)
    {
        n += 2;
    }

};

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

TEST_CASE("basic_message")
{
    object o;
    mutate(o).add<type_checker>();

    CHECK(get_self(o) == o.get<type_checker>());

    // works as ptr too
    CHECK(get_self(&o) == o.get<type_checker>());

    // issue #20
    CHECK(inherited(o) == 22);
}

TEST_CASE("complex_apply_mutation")
{
    object o;

    CHECK(!o.has<counter>());
    CHECK_FALSE(o.get<counter>());
    CHECK(!o.implements(dummy_msg));
    CHECK(!o.implements(unused_msg));
    CHECK(o.num_implementers(dummy_msg) == 0);

    single_object_mutator mutation(o);

    mutation.add<no_messages>();
    mutation.apply();
    CHECK(o.has<no_messages>());
    CHECK(o.get<no_messages>());
    CHECK(!o.implements(dummy_msg));
    CHECK(o.num_implementers(dummy_msg) == 0);

    mutation.add<counter>();
    mutation.apply();

    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    CHECK(o.implements(dummy_msg));
    CHECK(o.num_implementers(dummy_msg) == 1);
    CHECK(o.get<counter>()->get_count() == 0);
    o.get<counter>()->count_uni();
    CHECK(o.get<counter>()->get_count() == 1);

    mutation.remove<counter>();
    mutation.cancel();
    mutation.apply();
    // cancelled mutations should do nothing
    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    CHECK(o.get<counter>()->get_count() == 1);

    mutation.add<counter>();
    mutation.remove<counter>();
    mutation.apply();
    // adding and removing the same thing should do nothing
    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    CHECK(o.get<counter>()->get_count() == 1);

    mutation.add<counter>();
    mutation.apply();
    // adding something that's there should be fine
    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    // adding something that's already there shouldn't recreate the mixin
    CHECK(o.get<counter>()->get_count() == 1);

    mutation.remove<counter>();
    mutation.apply();
    CHECK(!o.has<counter>());
    CHECK_FALSE(o.get<counter>());

    CHECK(!o.implements(dummy_msg));
    CHECK(o.num_implementers(dummy_msg) == 0);
}

TEST_CASE("multicast")
{
    object o;

    mutate(o)
        .add<counter>();

    int n = 0;
    multi(o, n);
    CHECK(n == 1);

    mutate(o)
        .add<type_checker>();

    multi(o, n);
    CHECK(n == 4);

    mutate(o)
        .remove<counter>();
    multi(o, n);
    CHECK(n == 6);
}

#include <dynamix/define_mixin.hpp>

DYNAMIX_DEFINE_MIXIN(no_messages, none);
DYNAMIX_DEFINE_MIXIN(counter, dummy_msg & multi_msg);
DYNAMIX_DEFINE_MIXIN(type_checker, get_self_msg & multi_msg & inherited_msg);
