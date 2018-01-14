// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <dynamix/gen/no_arity_message_macros.hpp>
#include <iostream>

DYNAMIX_DECLARE_MIXIN(ma);
DYNAMIX_DECLARE_MIXIN(mb);

//
DYNAMIX_MESSAGE_1(int, add2, int, n);
DYNAMIX_MULTICAST_MESSAGE_1(int, add1, int, n);

int main()
{
    using namespace dynamix;
    using namespace std;

    object o;

    mutate(o)
        .add<ma>()
        .add<mb>();

    cout << add2(o, 5) << endl;
    cout << add1<combinators::sum>(o, 3) << endl;
    //add1(o, 3);

    return 0;
}


class ma
{
public:
    int add2(int n)
    {
        return n + 2;
    }

    int add1(int n)
    {
        return n + 1;
    }
};

class mb
{
public:
    int add1(int n)
    {
        return n - 1; // haha
    }
};

DYNAMIX_DEFINE_MESSAGE(add1);
DYNAMIX_DEFINE_MESSAGE(add2);

DYNAMIX_DEFINE_MIXIN(ma, add2_msg & add1_msg);
DYNAMIX_DEFINE_MIXIN(mb, add1_msg);
