// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "perf.hpp"

size_t A_LOT = 10000000;

int OBJ_NUM = 100000;

using namespace dynamix;
using namespace std;

abstract_class** ac_instances;
std::function<void(int)>* f_add;
std::function<int()>* f_sum;
std::function<void()>* f_noop;
dynamix::object** dm_objects;
regular_class* regular_objects;

DYNAMIX_DECLARE_MIXIN(regular_class);
DYNAMIX_DECLARE_MIXIN(regular_class2);

class abstract_instance : public abstract_class
{
public:
    abstract_instance() : _sum(0) {}

    void add(int i)
    {
        _sum+=i;
    }

    int sum() const
    {
        return _sum;
    }

    void noop() const {}

    int _sum;
};

class abstract_instance2 : public abstract_class
{
public:
    abstract_instance2() : _sum(0) {}

    void add(int i)
    {
        _sum+=i;
    }

    int sum() const
    {
        return _sum;
    }

    void noop() const {}

    int _sum;
};

class regular_class2
{
public:
    regular_class2() : _sum(0) {}

    void add(int i);

    int sum() const;

    void noop() const;

private:
    int _sum;
};


extern void initialize_globals()
{
    // don't care about memory leaks

    regular_objects = new regular_class[OBJ_NUM];

    ac_instances = new abstract_class*[OBJ_NUM];

    f_add = new std::function<void(int)>[OBJ_NUM];
    f_sum = new std::function<int()>[OBJ_NUM];
    f_noop = new std::function<void()>[OBJ_NUM];
    regular_class* objs = new regular_class[OBJ_NUM];
    regular_class2* objs2 = new regular_class2[OBJ_NUM];

    dm_objects = new object*[OBJ_NUM];

    for(int i=0; i<OBJ_NUM; ++i)
    {
        abstract_class* c;
        dm_objects[i] = new object;

        if(rand()%2)
        {
            c = new abstract_instance;

            f_add[i] = std::bind(&regular_class::add, objs + i, placeholders::_1);
            f_sum[i] = std::bind(&regular_class::sum, objs + i);
            f_noop[i] = std::bind(&regular_class::noop, objs + i);

            mutate(dm_objects[i]).add<regular_class>();
        }
        else
        {
            c = new abstract_instance2;

            f_add[i] = std::bind(&regular_class2::add, objs2 + i, placeholders::_1);
            f_sum[i] = std::bind(&regular_class2::sum, objs2 + i);
            f_noop[i] = std::bind(&regular_class2::noop, objs2 + i);

            mutate(dm_objects[i]).add<regular_class2>();
        }

        ac_instances[i] = c;
    }
}

//////////////////////////////////////////////////////

void regular_class::add(int i)
{
    _sum += i;
}

int regular_class::sum() const
{
    return _sum;
}

void regular_class::noop() const {}

void regular_class2::add(int i)
{
    _sum += i;
}

int regular_class2::sum() const
{
    return _sum;
}

void regular_class2::noop() const {}

DYNAMIX_DEFINE_MIXIN(regular_class, add_msg & sum_msg & noop_msg);
DYNAMIX_DEFINE_MIXIN(regular_class2, add_msg & sum_msg & noop_msg);

DYNAMIX_DEFINE_MESSAGE(add);
DYNAMIX_DEFINE_MESSAGE(sum);
DYNAMIX_DEFINE_MESSAGE(noop);
