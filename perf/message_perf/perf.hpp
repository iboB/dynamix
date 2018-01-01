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

//////////////////////////////////
// virtual dispatch

class abstract_class
{
public:
    virtual ~abstract_class() {}
    virtual void add(int) = 0;
    virtual int sum() const = 0;

    virtual void noop() const = 0;
};

abstract_class* new_abstract_class(int id);

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

//////////////////////////////////
// dynamix

dynamix::object new_object(int id);

DYNAMIX_MESSAGE_1(void, add, int, val);
DYNAMIX_CONST_MESSAGE_0(int, sum);
DYNAMIX_CONST_MESSAGE_0(void, noop);

//////////////////////////////////
