// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_SUITE("allocators");

using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(normal_a);
DYNAMIX_DECLARE_MIXIN(normal_b);
DYNAMIX_DECLARE_MIXIN(custom_1);
DYNAMIX_DECLARE_MIXIN(custom_2_a);
DYNAMIX_DECLARE_MIXIN(custom_2_b);
DYNAMIX_DECLARE_MIXIN(custom_own_var);

template <typename T>
struct alloc_counter
{
    static size_t data_allocations;
    static size_t data_deallocations;
    static size_t mixin_allocations;
    static size_t mixin_deallocations;
};

template <typename T>
size_t alloc_counter<T>::data_allocations;
template <typename T>
size_t alloc_counter<T>::data_deallocations;
template <typename T>
size_t alloc_counter<T>::mixin_allocations;
template <typename T>
size_t alloc_counter<T>::mixin_deallocations;


template <typename T>
struct custom_allocator : public mixin_allocator, public alloc_counter<T>
{
    // allocate memory for count mixin_data_in_object instances
    virtual char* alloc_mixin_data(size_t count) override
    {
        ++alloc_counter<T>::data_allocations;
        return new char[count * mixin_data_size];
    }

    virtual void dealloc_mixin_data(char* ptr) override
    {
        ++alloc_counter<T>::data_deallocations;
        delete[] ptr;
    }

    virtual void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer, size_t& out_mixin_offset) override
    {
        ++alloc_counter<T>::mixin_allocations;
        _dda.alloc_mixin(mixin_size, mixin_alignment, out_buffer, out_mixin_offset);
    }

    virtual void dealloc_mixin(char* ptr) override
    {
        ++alloc_counter<T>::mixin_deallocations;
        _dda.dealloc_mixin(ptr);
    }

    internal::default_allocator _dda;
};

class global_alloc : public custom_allocator<global_alloc> {};
class custom_alloc_1 : public custom_allocator<custom_alloc_1> {};
class custom_alloc_2 : public custom_allocator<custom_alloc_2> {};
class custom_alloc_var : public custom_allocator<custom_alloc_var> {} the_allocator;

TEST_CASE("allocators")
{
    global_alloc* glob = new global_alloc;
    set_global_allocator(glob);

    {
        object o;
        mutate(o)
            .add<normal_a>()
            .add<normal_b>()
            .add<custom_1>()
            .add<custom_2_a>()
            .add<custom_2_b>()
            .add<custom_own_var>();

        CHECK(alloc_counter<global_alloc>::data_allocations == 1);
        CHECK(alloc_counter<global_alloc>::mixin_allocations == 2); // two global mixins
        CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 1); // one of these
        CHECK(alloc_counter<custom_alloc_2>::mixin_allocations == 2); // two of these
        CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 1); // one of these
    }

    CHECK(alloc_counter<global_alloc>::data_deallocations == 1);
    CHECK(alloc_counter<global_alloc>::mixin_deallocations == 2); // two global mixins
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 1); // one of these
    CHECK(alloc_counter<custom_alloc_2>::mixin_deallocations == 2); // two of these
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 1); // one of these

    {
        object o1;
        mutate(o1)
            .add<normal_a>()
            .add<custom_1>()
            .add<custom_2_a>();

        object o2;
        mutate(o2)
            .add<normal_a>()
            .add<custom_1>()
            .add<custom_2_a>();

        object o3;
        mutate(o3)
            .add<normal_b>()
            .add<custom_2_b>();

        mutate(o1)
            .remove<normal_a>()
            .add<normal_b>()
            .remove<custom_2_a>()
            .add<custom_2_b>();

        CHECK(alloc_counter<global_alloc>::data_allocations == 5); // 1 + 4 new objects
        CHECK(alloc_counter<global_alloc>::data_deallocations == 2); // 1 + 1 changed object

        CHECK(alloc_counter<global_alloc>::mixin_allocations == 6); // 2 + 4
        CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 3); // 1 + 3
        CHECK(alloc_counter<custom_alloc_2>::mixin_allocations == 6); // 2 + 4
        CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 1); // 1 + 0
    }

    CHECK(alloc_counter<global_alloc>::data_deallocations == 5);

    CHECK(alloc_counter<global_alloc>::mixin_allocations == 6); // 2 + 4
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 3); // 1 + 3
    CHECK(alloc_counter<custom_alloc_2>::mixin_allocations == 6); // 2 + 4
    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 1); // 1 + 0

    // none of these should have had data allocations
    CHECK(alloc_counter<custom_alloc_1>::data_allocations == 0);
    CHECK(alloc_counter<custom_alloc_2>::data_allocations == 0);
    CHECK(alloc_counter<custom_alloc_var>::data_allocations == 0);
}

class normal_a {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_a"; }
#endif
};
class normal_b {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_b"; }
#endif
};
class custom_1 {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_1"; }
#endif
};
class custom_2_a {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_2_a"; }
#endif
};
class custom_2_b {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_2_b"; }
#endif
};
class custom_own_var {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_own_var"; }
#endif
};

DYNAMIX_DEFINE_MIXIN(normal_a, dynamix::none);
DYNAMIX_DEFINE_MIXIN(normal_b, dynamix::none);
DYNAMIX_DEFINE_MIXIN(custom_1, dynamix::allocator<custom_alloc_1>());
DYNAMIX_DEFINE_MIXIN(custom_2_a, dynamix::allocator<custom_alloc_2>());
DYNAMIX_DEFINE_MIXIN(custom_2_b, dynamix::allocator<custom_alloc_2>());
DYNAMIX_DEFINE_MIXIN(custom_own_var, the_allocator);
