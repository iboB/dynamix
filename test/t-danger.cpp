// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <doctest/doctest.h>

DYNAMIX_DECLARE_DOMAIN(test);

class test_obj;

DYNAMIX_DECLARE_MSG(mutate_self_msg, mutate_self, void, (test_obj&));

class test_obj : public dynamix::object {
public:
    int data;

    test_obj(int data) : dynamix::object(dynamix::g::get_domain<test>()), data(data) {}

    static test_obj* of(void* mixin) {
        return static_cast<test_obj*>(dynamix::object_of(mixin));
    }
    static const test_obj* of(const void* mixin) {
        return static_cast<const test_obj*>(dynamix::object_of(mixin));
    }
};

#define t_this ::test_obj::of(this)

DYNAMIX_DECLARE_MIXIN(struct access_object);
DYNAMIX_DECLARE_MIXIN(struct dummy);

struct access_object {
    int change;

    access_object(int expected, int change) : change(change) {
        CHECK(t_this->data == expected);
        t_this->data = change;
    }

    ~access_object() {
        CHECK(t_this->data == change);
    }
};

TEST_CASE("access object") {
    test_obj obj(5);
    mutate(obj, dynamix::add<access_object>(5, 10));
    CHECK(obj.data == 10);
    mutate(obj, dynamix::add<dummy>());
    CHECK(obj.data == 10);
}

DYNAMIX_DECLARE_MIXIN(struct self_mutator);

TEST_CASE("mutate self") {
    test_obj obj(10);
    mutate(obj, dynamix::add<self_mutator>());
    mutate_self(obj);
    CHECK(obj.has<dummy>());
}

struct self_mutator {
    void mutate_self() {
        mutate(*t_this, dynamix::add<dummy>());
    }
};

struct dummy {};

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

DYNAMIX_DEFINE_DOMAIN(test);
DYNAMIX_DEFINE_MIXIN(test, access_object);
DYNAMIX_DEFINE_MIXIN(test, self_mutator).implements<mutate_self_msg>();
DYNAMIX_DEFINE_MIXIN(test, dummy);

DYNAMIX_MAKE_FUNC_TRAITS(mutate_self);
DYNAMIX_DEFINE_MSG(mutate_self_msg, unicast, mutate_self, void, (test_obj&));
