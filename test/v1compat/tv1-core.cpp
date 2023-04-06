// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("v1 core");

using namespace dynamix::v1compat;

// some mixins and messages
DYNAMIX_V1_DECLARE_MIXIN(counter);
DYNAMIX_V1_DECLARE_MIXIN(no_messages);
DYNAMIX_V1_DECLARE_MIXIN(type_checker);

DYNAMIX_V1_MESSAGE_0(void, dummy);
DYNAMIX_V1_CONST_MESSAGE_0(const void*, get_self);
DYNAMIX_V1_MESSAGE_0(void, unused);
DYNAMIX_V1_MULTICAST_MESSAGE_1(void, multi, int&, n);
DYNAMIX_V1_MESSAGE_0(int, inherited);

class no_messages {};

class counter {
public:
    counter()
        : _count(0)
    {}

    void dummy() {}

    void count_uni() { ++_count; }
    void count_multi();

    int get_count() const { return _count; }

    void multi(int& n) {
        ++n;
    }

private:
    int _count;
};

// issue #20 test
struct parent {
    int inherited() {
        return 22;
    }
};

class type_checker : public parent {
public:
    const void* get_self() const {
        return this;
    }

    void multi(int& n) {
        n += 2;
    }
};

TEST_CASE("mixin_type_info") {
    {
        auto& info = dynamix::g::get_mixin_info<no_messages>();
        CHECK(info.registered());
        CHECK(info.user_data == 0);
        CHECK(info.features_span().empty());
    }

    {
        auto& info = dynamix::g::get_mixin_info<counter>();
        CHECK(info.registered());
        CHECK(info.user_data == 33);
        CHECK(info.features_span().size() == 2);
    }

    {
        auto& info = dynamix::g::get_mixin_info<type_checker>();
        CHECK(info.registered());
        CHECK(info.user_data == 44);
        CHECK(info.features_span().size() == 3);
    }
}

TEST_CASE("simple_inline_mutation") {
    object o;

    CHECK(o.empty());
    CHECK(!o.has<no_messages>());
    CHECK_FALSE(o.get<no_messages>());

    mutate(o).add<no_messages>();

    CHECK(o.has<no_messages>());
    CHECK(o.get<no_messages>());

    mutate(o).remove<no_messages>();

    CHECK(!o.has<no_messages>());
    CHECK_FALSE(o.get<no_messages>());
    CHECK(o.empty());

    mutate(o).add<no_messages>();
    o.clear();

    CHECK(!o.has<no_messages>());
    CHECK_FALSE(o.get<no_messages>());
    CHECK(o.empty());
}

TEST_CASE("basic_message") {
    object o;
    mutate(o).add<type_checker>();

    CHECK(get_self(o) == o.get<type_checker>());

    // no longer supported:
    // works as ptr too
    // CHECK(get_self(&o) == o.get<type_checker>());

    // issue #20
    CHECK(inherited(o) == 22);
}

TEST_CASE("complex_apply_mutation") {
    object o;

    CHECK(!o.has<counter>());
    CHECK_FALSE(o.get<counter>());
    CHECK(!o.implements(dummy_msg));
    CHECK(!o.implements(unused_msg));

    mutate(o).add<no_messages>();
    CHECK(o.has<no_messages>());
    CHECK(o.get<no_messages>());
    CHECK(!o.implements(dummy_msg));

    mutate(o).add<counter>();

    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    CHECK(o.implements(dummy_msg));
    CHECK(o.get<counter>()->get_count() == 0);
    o.get<counter>()->count_uni();
    CHECK(o.get<counter>()->get_count() == 1);

    mutate(o).add<counter>().remove<counter>();
    // adding and removing the same thing should do nothing
    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    CHECK(o.get<counter>()->get_count() == 1);

    // v2!: add_if_lacking
    mutate(o).add_if_lacking<counter>();

    CHECK(o.has<counter>());
    CHECK(o.get<counter>());
    // adding something that's already there shouldn't recreate the mixin
    CHECK(o.get<counter>()->get_count() == 1);

    mutate(o).remove<counter>();
    CHECK(!o.has<counter>());
    CHECK_FALSE(o.get<counter>());

    CHECK(!o.implements(dummy_msg));
}

TEST_CASE("multicast") {
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

TEST_CASE("type_template") {
    // v2!: no more type templates
    auto& dom = domain::instance();
    dynamix::type_mutation mut(dom);
    mut.add<counter>();
    mut.add<no_messages>();

    const dynamix::type& type = dom.get_type(std::move(mut));

    object o1(type);
    CHECK(o1.has<no_messages>());
    CHECK(o1.get<no_messages>());
    CHECK(o1.has<counter>());
    CHECK(o1.get<counter>());
    CHECK(o1.implements(dummy_msg));

    object o2;
    o2.reset_type(type);
    CHECK(o2.has<no_messages>());
    CHECK(o2.get<no_messages>());
    CHECK(o2.has<counter>());
    CHECK(o2.get<counter>());
    CHECK(o2.implements(dummy_msg));

    object o3;
    mutate(o3)
        .add<counter>();
    o3.get<counter>()->count_uni();
    CHECK(o3.get<counter>()->get_count() == 1);
    o3.reset_type(type);
    // v2!: applying a type template no longer should resets the object
    CHECK(o3.get<counter>()->get_count() == 1);
}

DYNAMIX_V1_DEFINE_MIXIN(no_messages, none);
DYNAMIX_V1_DEFINE_MIXIN(counter, dummy_msg & multi_msg & user_data(33));
DYNAMIX_V1_DEFINE_MIXIN(type_checker, get_self_msg & user_data(44) & multi_msg & inherited_msg);

DYNAMIX_V1_DEFINE_MESSAGE(dummy);
DYNAMIX_V1_DEFINE_MESSAGE(get_self);
DYNAMIX_V1_DEFINE_MESSAGE(unused);
DYNAMIX_V1_DEFINE_MESSAGE(multi);
DYNAMIX_V1_DEFINE_MESSAGE(inherited);
