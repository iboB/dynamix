// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <string>
#include <sstream>
#include <dynamix/dynamix.hpp>
#include <dynamix/gen/no_arity_message_macros.hpp>

using namespace std;
using namespace dynamix;

using namespace dynamix;

// some mixins and messages
DYNAMIX_DECLARE_MIXIN(counter);
DYNAMIX_DECLARE_MIXIN(no_messages);
DYNAMIX_DECLARE_MIXIN(type_checker);
//DYNAMIX_DECLARE_MIXIN(overrider);
//DYNAMIX_DECLARE_MIXIN(foo);
//DYNAMIX_DECLARE_MIXIN(bar);

DYNAMIX_MESSAGE(void, dummy);
DYNAMIX_CONST_MESSAGE(const void*, get_self);

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
private:
    int _count;
};

class type_checker
{
public:
    const void* get_self() const
    {
        return this;
    }
};

int main()
{
    object o;
    mutate(o).add<type_checker>();

    cout << (get_self(o) == o.get<type_checker>()) << endl;

    // works as ptr too
    cout << (get_self(&o) == o.get<type_checker>()) << endl;

    return 0;
}

DYNAMIX_DEFINE_MIXIN(no_messages, none);
DYNAMIX_DEFINE_MIXIN(counter, dummy_msg);
DYNAMIX_DEFINE_MIXIN(type_checker, get_self_msg);

DYNAMIX_DEFINE_MESSAGE(dummy);
DYNAMIX_DEFINE_MESSAGE(get_self);
