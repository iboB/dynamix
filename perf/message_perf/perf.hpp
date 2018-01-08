// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include <iostream>

#define DYNAMIX_NO_MSG_THROW
#include <dynamix/dynamix.hpp>

#include <functional>
#include <vector>

//////////////////////////////////
// working data
std::vector<int>& random_ints();
std::vector<unsigned>& random_ints_partial_sums();

void fill_sample_data(size_t max_size);

//////////////////////////////////
// virtual dispatch

class abstract_class
{
public:
    virtual ~abstract_class() {}
    virtual void add(int) = 0;
    virtual int sum() const = 0;

    virtual void noop() const = 0;

    virtual void multi_add(int i) = 0;
    virtual int multi_sum() const = 0;
    virtual void multi_noop() const = 0;
};

abstract_class* new_abstract_class(int id);

struct abstract_multi
{
    abstract_multi() = default;
    abstract_multi(const abstract_multi&) = delete;
    abstract_multi(abstract_multi&& other)
        : funcs(std::move(other.funcs))
    {}
    ~abstract_multi();

    unsigned sum() const
    {
        unsigned s = 0;
        for (auto f : funcs)
        {
            s += f->sum();
        }
        return s;
    }
    void add(int i)
    {
        for (auto f : funcs)
        {
            f->add(i);
        }
    }
    void noop()
    {
        for (auto f : funcs)
        {
            f->noop();
        }
    }
    std::vector<abstract_class*> funcs;
};

abstract_multi new_abstract_multi(int id);

//////////////////////////////////
// std function

struct std_func_object
{
    std::function<void(int)> add;
    std::function<int()> sum;
    std::function<void()> noop;
    std::function<void()> release;
};

std_func_object new_std_func(int id);

struct std_func_multi
{
    std::vector<std::function<void(int)>> adds;
    std::vector<std::function<unsigned()>> sums;
    std::vector<std::function<void()>> noops;
    std::vector<std::function<void()>> release;
};

std_func_multi new_std_multi(int id);

//////////////////////////////////
// dynamix

dynamix::object new_object(int id);
dynamix::object new_multi_object(int id);

DYNAMIX_MESSAGE_1(void, add, int, val);
DYNAMIX_CONST_MESSAGE_0(int, sum);
DYNAMIX_CONST_MESSAGE_0(void, noop);

DYNAMIX_MULTICAST_MESSAGE_1(void, multi_add, int, val);
DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, multi_sum_out, unsigned&, out);
DYNAMIX_CONST_MULTICAST_MESSAGE_0(unsigned, multi_sum);
DYNAMIX_CONST_MULTICAST_MESSAGE_0(void, multi_noop);

//////////////////////////////////
