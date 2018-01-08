// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "perf.hpp"

#include <cstdlib>

using namespace dynamix;
using namespace std;

static vector<int> ints;
static vector<unsigned> partials;
std::vector<int>& random_ints() { return ints; }
std::vector<unsigned>& random_ints_partial_sums() { return partials; }

void fill_sample_data(size_t max_size)
{
    unsigned sum = 0;
    ints.reserve(max_size);
    partials.reserve(max_size);

    for (size_t i = 0; i < max_size; ++i)
    {
        int n = rand() - RAND_MAX / 2;
        sum += n;
        ints.push_back(n);
        partials.push_back(sum);
    }
}

DYNAMIX_DECLARE_MIXIN(regular_class);
DYNAMIX_DECLARE_MIXIN(regular_class2);
DYNAMIX_DECLARE_MIXIN(multi_class);
DYNAMIX_DECLARE_MIXIN(multi_class2);

class abstract_instance : public abstract_class
{
public:
    abstract_instance() : _sum(0) {}

    void add(int i) override
    {
        _sum+=i;
    }

    int sum() const override
    {
        return _sum;
    }

    void noop() const override {}

    void multi_add(int i) override
    {
        _sum += i;
    }

    int multi_sum() const override
    {
        return _sum;
    }

    void multi_noop() const override {}

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

    void multi_add(int i) override
    {
        _sum += i;
    }

    int multi_sum() const override
    {
        return _sum;
    }

    void multi_noop() const override {}

    int _sum;
    int _unused = 4;
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

abstract_multi::~abstract_multi()
{
    for (auto f : funcs)
    {
        delete f;
    }
}

abstract_multi new_abstract_multi(int id)
{
    abstract_multi ret;

    switch (id % 4)
    {
    case 0:
        ret.funcs.push_back(new abstract_instance);
        ret.funcs.push_back(new abstract_instance2);
        ret.funcs.push_back(new abstract_instance);
        break;
    case 1:
        ret.funcs.push_back(new abstract_instance2);
        ret.funcs.push_back(new abstract_instance2);
        ret.funcs.push_back(new abstract_instance);
        break;
    case 2:
        ret.funcs.push_back(new abstract_instance);
        ret.funcs.push_back(new abstract_instance2);
        ret.funcs.push_back(new abstract_instance2);
        break;
    case 3:
        ret.funcs.push_back(new abstract_instance2);
        ret.funcs.push_back(new abstract_instance);
        ret.funcs.push_back(new abstract_instance2);
        break;
    }
    return ret;
}

//////////////////////////////////////////////////////

class regular_class
{
public:
    regular_class() : _sum(0) {}

    void add(int i);
    int sum() const;
    void noop() const;
    void multi_add(int i);
    void multi_sum_out(unsigned& out) const;
    unsigned multi_sum() const;
    void multi_noop() const;

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
    
    void multi_add(int i);
    void multi_sum_out(unsigned& out) const;
    unsigned multi_sum() const;
    void multi_noop() const;

private:
    int _sum;
    int _unused = 33;
};

class multi_class
{
public:
    void multi_add(int i);
    void multi_sum_out(unsigned& out) const;
    unsigned multi_sum() const;
    void multi_noop() const;
private:
    int _sum = 0;
    int _unused = 33;
};

class multi_class2
{
public:
    void multi_add(int i);
    void multi_sum_out(unsigned& out) const;
    unsigned multi_sum() const;
    void multi_noop() const;
private:
    int _sum = 0;
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

template <typename T>
std_func_object new_std_func_multi_obj()
{
    auto instance = new T;

    std_func_object ret;
    ret.add = std::bind(&T::multi_add, instance, placeholders::_1);
    ret.sum = std::bind(&T::multi_sum, instance);
    ret.noop = std::bind(&T::multi_noop, instance);
    ret.release = std::bind(release_rc<T>, instance);

    return ret;
}

std_func_multi new_std_multi(int id)
{
    vector<std_func_object> data;
    switch (id % 4)
    {
    case 0:
        data.emplace_back(new_std_func_multi_obj<regular_class>());
        data.emplace_back(new_std_func_multi_obj<regular_class2>());
        data.emplace_back(new_std_func_multi_obj<multi_class>());
        break;
    case 1:
        data.emplace_back(new_std_func_multi_obj<regular_class>());
        data.emplace_back(new_std_func_multi_obj<multi_class2>());
        data.emplace_back(new_std_func_multi_obj<multi_class>());
        break;
    case 2:
        data.emplace_back(new_std_func_multi_obj<regular_class2>());
        data.emplace_back(new_std_func_multi_obj<multi_class2>());
        data.emplace_back(new_std_func_multi_obj<multi_class>());
        break;
    case 3:
        data.emplace_back(new_std_func_multi_obj<regular_class>());
        data.emplace_back(new_std_func_multi_obj<regular_class2>());
        data.emplace_back(new_std_func_multi_obj<multi_class2>());
        break;
    }

    std_func_multi ret;
    ret.adds.reserve(data.size());
    ret.noops.reserve(data.size());
    ret.sums.reserve(data.size());
    ret.release.reserve(data.size());
    for (auto& d : data)
    {
        ret.adds.emplace_back(std::move(d.add));
        ret.noops.emplace_back(std::move(d.noop));
        ret.sums.emplace_back(std::move(d.sum));
        ret.release.emplace_back(std::move(d.release));
    }
    return ret;
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

object new_multi_object(int id)
{
    object ret;
    switch (id % 4)
    {
    case 0:
        mutate(ret)
            .add<regular_class>()
            .add<regular_class2>()
            .add<multi_class>();
        break;
    case 1:
        mutate(ret)
            .add<regular_class>()
            .add<regular_class2>()
            .add<multi_class2>();
        break;
    case 2:
        mutate(ret)
            .add<regular_class>()
            .add<multi_class>()
            .add<multi_class2>();
        break;
    case 3:
        mutate(ret)
            .add<regular_class2>()
            .add<multi_class>()
            .add<multi_class2>();
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

void regular_class2::multi_add(int i)
{
    _sum += i;
}

void regular_class2::multi_sum_out(unsigned& out) const
{
    out += _sum;
}

unsigned regular_class2::multi_sum() const
{
    return _sum;
}

void regular_class2::multi_noop() const {}

void regular_class::multi_add(int i)
{
    _sum += i;
}

void regular_class::multi_sum_out(unsigned& out) const
{
    out += _sum;
}

unsigned regular_class::multi_sum() const
{
    return _sum;
}

void regular_class::multi_noop() const {}

void multi_class::multi_add(int i)
{
    _sum += i;
}

void multi_class::multi_sum_out(unsigned& out) const
{
    out += _sum;
}

unsigned multi_class::multi_sum() const
{
    return _sum;
}

void multi_class::multi_noop() const {}

void multi_class2::multi_add(int i)
{
    _sum += i;
}

void multi_class2::multi_sum_out(unsigned& out) const
{
    out += _sum;
}

unsigned multi_class2::multi_sum() const
{
    return _sum;
}

void multi_class2::multi_noop() const {}


DYNAMIX_DEFINE_MIXIN(regular_class, priority(-1, add_msg) & priority(-1, sum_msg) & priority(-1, noop_msg) & multi_noop_msg & multi_add_msg & multi_sum_msg & multi_sum_out_msg);
DYNAMIX_DEFINE_MIXIN(regular_class2, add_msg & sum_msg & noop_msg & multi_noop_msg & multi_add_msg & multi_sum_msg & multi_sum_out_msg);
DYNAMIX_DEFINE_MIXIN(multi_class, multi_noop_msg & multi_add_msg & multi_sum_msg & multi_sum_out_msg);
DYNAMIX_DEFINE_MIXIN(multi_class2, multi_noop_msg & multi_add_msg & multi_sum_msg & multi_sum_out_msg);

DYNAMIX_DEFINE_MESSAGE(add);
DYNAMIX_DEFINE_MESSAGE(sum);
DYNAMIX_DEFINE_MESSAGE(noop);

DYNAMIX_DEFINE_MESSAGE(multi_add);
DYNAMIX_DEFINE_MESSAGE(multi_sum);
DYNAMIX_DEFINE_MESSAGE(multi_sum_out);
DYNAMIX_DEFINE_MESSAGE(multi_noop);

