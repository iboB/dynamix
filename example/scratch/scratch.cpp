// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#define DYNAMIX_USE_LEGACY_MESSAGE_MACROS
#include <dynamix/dynamix.hpp>

#include <iostream>

using namespace std;

DYNAMIX_MULTICAST_MESSAGE_1(void, testv, vector<int>, data);

class a
{
public:
    void testv(vector<int> data)
    {
        cout << "a: " << data.size() << endl;
    }
};

DYNAMIX_DEFINE_MIXIN(a, testv_msg);

class b
{
public:
    void testv(vector<int> data)
    {
        cout << "b: " << data.size() << endl;
    }
};

DYNAMIX_DEFINE_MIXIN(b, testv_msg);


int main()
{
    auto hero = new dynamix::object;
    dynamix::mutate(hero)
        .add<a>()
        .add<b>();

    vector<int> foo = {1, 2, 3};
    testv(hero, foo);

    return 0;
}

DYNAMIX_DEFINE_MESSAGE(testv);
