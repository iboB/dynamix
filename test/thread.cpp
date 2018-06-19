// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include <thread>
#include <vector>
#include <random>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_SUITE("thread");

using namespace dynamix;

const int A_LOT = 1000;

DYNAMIX_DECLARE_MIXIN(m1);
DYNAMIX_DECLARE_MIXIN(m2);
DYNAMIX_DECLARE_MIXIN(m3);

DYNAMIX_DECLARE_MIXIN(e1);
DYNAMIX_DECLARE_MIXIN(e2);

DYNAMIX_MESSAGE_1(void, setter1, int, n);
DYNAMIX_MESSAGE_1(void, setter2, int, n);
DYNAMIX_MESSAGE_1(void, setter3, int, n);
DYNAMIX_CONST_MESSAGE_0(int, getter1);
DYNAMIX_CONST_MESSAGE_0(int, getter2);
DYNAMIX_CONST_MESSAGE_0(int, getter3);
DYNAMIX_CONST_MULTICAST_MESSAGE_0(int, getter);

template <void Msg(object&, int)>
void set_ints(std::vector<object>& objects)
{
    std::minstd_rand gen(1234);
    std::uniform_int_distribution<> rnd(0, 1000);

    for (auto& obj : objects)
    {
        if (!obj.get<m2>()) return;
        Msg(obj, rnd(gen));
    }
}

template <int Msg(const object&)>
void sum_ints(const std::vector<object>& objects, int& sum)
{
    for (auto& obj : objects)
    {
        if (!obj.has<m1>()) return;
        sum += Msg(obj);
    }
}

void sum_multi(const std::vector<object>& objects, int& sum)
{
    for (auto& obj : objects)
    {
        if (!obj.implements(getter_msg)) return;
        sum += getter<combinators::sum>(obj);
    }
}

TEST_CASE("msg_thread")
{
    std::vector<object> objects(A_LOT);

    for (auto& obj : objects)
    {
        mutate(obj).add<m1>().add<m2>().add<m3>();
    }

    std::thread set1(set_ints<setter1>, std::ref(objects));
    std::thread set2(set_ints<setter2>, std::ref(objects));
    std::thread set3(set_ints<setter3>, std::ref(objects));

    set1.join();
    set2.join();
    set3.join();

    int s1 = 0;
    int s2 = 0;
    int s3 = 0;
    std::thread sum3(sum_ints<getter3>, std::ref(objects), std::ref(s3));
    std::thread sum2(sum_ints<getter2>, std::ref(objects), std::ref(s2));
    std::thread sum1(sum_ints<getter1>, std::ref(objects), std::ref(s1));

    sum1.join();
    sum2.join();
    sum3.join();

    CHECK(s1 != 0);
    CHECK(s1 == s2);
    CHECK(s2 == s3);

    s1 = 0;
    s2 = 0;
    std::thread sum_multi1(sum_multi, std::ref(objects), std::ref(s1));
    std::thread sum_multi2(sum_multi, std::ref(objects), std::ref(s2));

    sum_multi1.join();
    sum_multi2.join();

    CHECK(s1 == s2);
    CHECK(s1 == 3 * s3);
}

void mutate_objects(int seed, std::vector<object>& objects)
{
    using namespace std;
    const mixin_id ids[] =
    {
        _dynamix_get_mixin_type_info((m1*)nullptr).id,
        _dynamix_get_mixin_type_info((m2*)nullptr).id,
        _dynamix_get_mixin_type_info((m3*)nullptr).id,
        _dynamix_get_mixin_type_info((e1*)nullptr).id,
        _dynamix_get_mixin_type_info((e2*)nullptr).id,
    };

    minstd_rand gen(seed);

    for (auto& obj : objects)
    {
        single_object_mutator m(obj);
        for (auto id : ids)
        {
            if(gen() % 2) continue;
            m.add(id);
        }
    }
}

#if DYNAMIX_THREAD_SAFE_MUTATIONS

TEST_CASE("mutate")
{
    using namespace std;
    vector<object> objectsa(A_LOT), objectsb(A_LOT),
        objectsc(A_LOT), objectsd(A_LOT);

    std::thread mut1(mutate_objects, 10, std::ref(objectsa));
    std::thread mut2(mutate_objects, 10, std::ref(objectsb));
    std::thread mut3(mutate_objects, 45, std::ref(objectsc));
    std::thread mut4(mutate_objects, 45, std::ref(objectsd));

    mut1.join();
    mut2.join();
    mut3.join();
    mut4.join();

    bool same = true;
    for (int i=0; i<A_LOT; ++i)
    {
        same = same && (objectsa[i]._type_info == objectsb[i]._type_info);
        same = same && (objectsc[i]._type_info == objectsd[i]._type_info);
    }
    CHECK(same);
}

#endif

class m1
{
public:
    #if !DYNAMIX_USE_TYPEID
        static const char* dynamix_mixin_name() { return "m1"; }
    #endif

    void setter1(int n)
    {
        _n = n;
    }

    int getter1() const
    {
        return _n;
    }

    int getter() const
    {
        return _n;
    }

private:
    int _n = 0;
};

class m2
{
public:
    #if !DYNAMIX_USE_TYPEID
        static const char* dynamix_mixin_name() { return "m2"; }
    #endif

    void setter2(int n)
    {
        _n = n;
    }

    int getter2() const
    {
        return _n;
    }

    int getter() const
    {
        return _n;
    }

private:
    int _n = 0;
};

class m3
{
public:
    #if !DYNAMIX_USE_TYPEID
        static const char* dynamix_mixin_name() { return "m3"; }
    #endif

    void setter3(int n)
    {
        _n = n;
    }

    int getter3() const
    {
        return _n;
    }

    int getter() const
    {
        return _n;
    }

private:
    int _n = 0;
};

class e1
{
public:
    #if !DYNAMIX_USE_TYPEID
        static const char* dynamix_mixin_name() { return "e1"; }
    #endif
};
class e2
{
public:
    #if !DYNAMIX_USE_TYPEID
        static const char* dynamix_mixin_name() { return "e2"; }
    #endif
};

DYNAMIX_DEFINE_MIXIN(m1, setter1_msg & getter1_msg & getter_msg);
DYNAMIX_DEFINE_MIXIN(m2, setter2_msg & getter2_msg & getter_msg);
DYNAMIX_DEFINE_MIXIN(m3, setter3_msg & getter3_msg & getter_msg);
DYNAMIX_DEFINE_MIXIN(e1, none);
DYNAMIX_DEFINE_MIXIN(e2, none);

DYNAMIX_DEFINE_MESSAGE(setter1);
DYNAMIX_DEFINE_MESSAGE(setter2);
DYNAMIX_DEFINE_MESSAGE(setter3);
DYNAMIX_DEFINE_MESSAGE(getter1);
DYNAMIX_DEFINE_MESSAGE(getter2);
DYNAMIX_DEFINE_MESSAGE(getter3);
DYNAMIX_DEFINE_MESSAGE(getter);
