// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "perf.hpp"

using namespace dynamix;
using namespace std;

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

abstract_class* new_abstract_class(int id)
{
    switch (id % 2)
    {
    case 0:
        return new abstract_instance;
    case 1:
        return new abstract_instance2;
    }
    assert(false);
    return nullptr;
}

class regular_class
{
public:
    regular_class() : _sum(0) {}

    void add(int i);
    int sum() const;

    void noop() const;

private:
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

//////////////////////////////////////////////////////

template <typename T>
static void release_rc(T* r)
{
    delete r;
}

template <typename T>
std_func_object new_std_func_object()
{
    auto instance = new T;

    std_func_object ret;
    ret.add = std::bind(&T::add, instance, placeholders::_1);
    ret.sum = std::bind(&T::sum, instance);
    ret.noop = std::bind(&T::noop, instance);
    ret.release = std::bind(release_rc<T>, instance);

    return ret;
}

std_func_object new_std_func(int id)
{
    switch (id % 2)
    {
    case 0:
        return new_std_func_object<regular_class>();
    case 1:
        return new_std_func_object<regular_class2>();
    }
    assert(false);
    return std_func_object();
}


//////////////////////////////////////////////////////

object new_object(int id)
{
    object ret;
    switch (id % 2)
    {
    case 0:
        mutate(ret).add<regular_class>();
        break;
    case 1:
        mutate(ret).add<regular_class2>();
        break;
    }
    return ret;
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
