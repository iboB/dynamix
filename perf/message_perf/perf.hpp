// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
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

extern size_t A_LOT; // intentionally not const
extern int OBJ_NUM; // number of objects

//////////////////////////////////
// regular method

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

extern regular_class* regular_objects;

//////////////////////////////////
// virtual dispatch

class abstract_class
{
public:
    virtual void add(int) = 0;
    virtual int sum() const = 0;

    virtual void noop() const = 0;
};

extern abstract_class** ac_instances;

//////////////////////////////////
// std function

extern std::function<void(int)>* f_add;
extern std::function<int()>* f_sum;
extern std::function<void()>* f_noop;

//////////////////////////////////
// dynamix

extern dynamix::object** dm_objects;

DYNAMIX_MESSAGE_1(void, add, int, val);
DYNAMIX_CONST_MESSAGE_0(int, sum);
DYNAMIX_CONST_MESSAGE_0(void, noop);

//////////////////////////////////

extern void initialize_globals();
