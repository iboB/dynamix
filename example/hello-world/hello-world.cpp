// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

#include <iostream>

DYNAMIX_DECLARE_MIXIN(struct hello);
DYNAMIX_DECLARE_MIXIN(struct bye);

DYNAMIX_DECLARE_MSG(speak_msg, speak, void, (const dynamix::object&));

DYNAMIX_DECLARE_DOMAIN(example);

int main() {
    dynamix::object greeter(dynamix::g::get_domain<example>());

    // Composition
    mutate(greeter, dynamix::add<hello>());

    // Message call
    speak(greeter);

    // Mutation: keep the interface, but change the implementation
    mutate(greeter,
        dynamix::remove<hello>(),
        dynamix::add<bye>()
    );

    // Message call (in mutated object)
    speak(greeter);

    return 0;
}

// The following code doesn't need to be in this compilation unit

struct hello {
    void speak() const {
        std::cout << "Hello from DynaMix!" << std::endl;
    }
};

struct bye {
    void speak() const {
        std::cout << "Good bye from DynaMix." << std::endl;
    }
};

// Mixin definitions
DYNAMIX_DEFINE_MIXIN(example, hello).implements<speak_msg>();
DYNAMIX_DEFINE_MIXIN(example, bye).implements<speak_msg>();

// Message definition
DYNAMIX_MAKE_FUNC_TRAITS(speak);
DYNAMIX_DEFINE_MSG(speak_msg, unicast, speak, void, (const dynamix::object&));

// Domain definition
DYNAMIX_DEFINE_DOMAIN(example);
