// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <iostream>

DYNAMIX_DECLARE_MIXIN(hello);
DYNAMIX_DECLARE_MIXIN(bye);

DYNAMIX_MESSAGE_0(void, speak);

int main()
{
    dynamix::object greeter;

    // Composition
    dynamix::mutate(greeter)
        .add<hello>();

    // Message call
    speak(greeter);

    // Mutation: keep the interface, but change the implementation
    dynamix::mutate(greeter)
        .remove<hello>()
        .add<bye>();

    // Message call (in mutated object)
    speak(greeter);

    return 0;
}

// The following code doesn't need to be in this compilation unit

class hello
{
public:
    void speak()
    {
        std::cout << "Hello from DynaMix" << std::endl;
    }
};

class bye
{
public:
    void speak()
    {
        std::cout << "And good bye from DynaMix" << std::endl;
    }
};

// Mixin definitions
DYNAMIX_DEFINE_MIXIN(hello, speak_msg);
DYNAMIX_DEFINE_MIXIN(bye, speak_msg);

// Message definition
DYNAMIX_DEFINE_MESSAGE(speak);