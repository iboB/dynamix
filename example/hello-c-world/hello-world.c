// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>
#include <dnmx/object.h>
#include <dnmx/type.h>
#include <dnmx/mutate.h>
#include <dnmx/mixin_info_util.h>

#include <stdio.h>

struct hello_domain_def {
    dnmx_domain_handle domain;
    dnmx_feature_info speak;
};

struct hello_domain_def hello_domain;

dnmx_mixin_info hello;
dnmx_mixin_info bye;

void speak(dnmx_object_handle obj);

void init_hello_domain(void);
void destroy_hello_domain(void);
void init_hello_mixins(void);

int main() {
    init_hello_domain();
    init_hello_mixins();
    dnmx_object_handle greeter = dnmx_create_object_empty(hello_domain.domain);

    // Composition
    {
        dnmx_mutate_op op = {.op_type = dnmx_mutate_op_add, .mixin = &hello};
        dnmx_mutate(greeter, &op, 1);
    }

    // Polymorphic call
    speak(greeter);

    // Mutation: keep the interface, but change the implementation
    {
        dnmx_mutate_op ops[] = {
            {.op_type = dnmx_mutate_op_remove, .mixin = &hello},
            {.op_type = dnmx_mutate_op_add, .mixin = &bye}
        };
        dnmx_mutate(greeter, ops, 2);
    }

    // Polymorphic call in mutated object
    speak(greeter);

    dnmx_destroy_object(greeter);
    destroy_hello_domain();
    return 0;
}

void init_hello_domain(void) {
    hello_domain.domain = dnmx_create_domain(dnmx_make_sv_lit("example"), (dnmx_domain_settings){0}, 0, NULL);
    hello_domain.speak = dnmx_make_feature_info();
    hello_domain.speak.name = dnmx_make_sv_lit("speak");
}

void say_hello(void) {
    puts("Hello from DynaMix!");
}
dnmx_feature_for_mixin hello_feature_for_mixin = {
    .info = &hello_domain.speak,
    .payload = (void*)(say_hello),
};

void say_bye(void) {
    puts("Good bye from DynaMix.");
}
dnmx_feature_for_mixin bye_feature_for_mixin = {
    .info = &hello_domain.speak,
    .payload = (void*)(say_bye),
};

void init_hello_mixins(void) {
    DNMX_COMMON_INIT_EMPTY_MI(hello, "hello");
    hello.features = &hello_feature_for_mixin;
    hello.num_features = 1;
    dnmx_register_mixin(hello_domain.domain, &hello);
    DNMX_COMMON_INIT_EMPTY_MI(bye, "bye");
    bye.features = &bye_feature_for_mixin;
    bye.num_features = 1;
    dnmx_register_mixin(hello_domain.domain, &bye);
}

void destroy_hello_domain(void) {
    dnmx_destroy_domain(hello_domain.domain);
}

typedef void (*vv_func)(void);

void speak(dnmx_object_handle obj) {
    dnmx_ftable_entry e = dnmx_ftable_at(obj->m_type, hello_domain.speak.id);
    vv_func func = (vv_func)e.begin->payload;
    func();
}
