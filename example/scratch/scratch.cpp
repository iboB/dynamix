// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <string>
#include <sstream>
#include <dynamix/dynamix.hpp>

using namespace std;
using namespace dynamix;

class trivial_copy
{
public:
    int i = 0;
};

DYNAMIX_DECLARE_MIXIN(trivial_copy);
DYNAMIX_DEFINE_MIXIN(trivial_copy, none);

class special_copy
{
public:
    special_copy() = default;
    special_copy(const special_copy& other)
        : i(other.i + 1)
        , cc(1)
    {}
    special_copy& operator=(const special_copy& other)
    {
        i = other.i + 2;
        ++a;
        return *this;
    }
    int i = 0;
    int cc = 0;
    int a = 0;
};

DYNAMIX_DECLARE_MIXIN(special_copy);
DYNAMIX_DEFINE_MIXIN(special_copy, none);

class no_copy
{
public:
    no_copy() = default;
    no_copy(const no_copy&) = delete;
    no_copy& operator=(const no_copy& other) = delete;
    int i = 0;
};

DYNAMIX_DECLARE_MIXIN(no_copy);
DYNAMIX_DEFINE_MIXIN(no_copy, none);

int main()
{
    object osrc;
    mutate(osrc)
        .add<trivial_copy>()
        .add<special_copy>();

    osrc.get<special_copy>()->i = 5;

    object o1 = osrc.copy();

    cout << o1.get<special_copy>()->i << endl;

    o1.copy_from(osrc);

    return 0;
}

