// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../test/doctest/doctest.h"

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

const object* the_object = nullptr;

template <typename T>
struct custom_allocator : public domain_allocator, public alloc_counter<T>
{
    // allocate memory for count mixin_data_in_object instances
    virtual char* alloc_mixin_data(size_t count, const object* obj) override
    {
        CHECK(obj == the_object);
        ++alloc_counter<T>::data_allocations;
        return new char[count * mixin_data_size];
    }

    virtual void dealloc_mixin_data(char* ptr, size_t count, const object* obj) override
    {
        CHECK((!the_object || obj == the_object));
        ++alloc_counter<T>::data_deallocations;
        delete[] ptr;
    }

    virtual std::pair<char*, size_t> alloc_mixin(const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(obj == the_object);
        ++alloc_counter<T>::mixin_allocations;
        return _dda.alloc_mixin(info, obj);
    }

    virtual void dealloc_mixin(char* ptr, size_t offset, const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(offset == mixin_offset(ptr, info.alignment));
        CHECK((!the_object || obj == the_object));
        ++alloc_counter<T>::mixin_deallocations;
        _dda.dealloc_mixin(ptr, offset, info, obj);
    }

    internal::default_allocator _dda;
};

class global_alloc : public custom_allocator<global_alloc> {};

class custom_alloc_1 : public custom_allocator<custom_alloc_1>
{
public:
    using super = custom_allocator<custom_alloc_1>;

    custom_alloc_1()
        : m_info(_dynamix_get_mixin_type_info((custom_1*)nullptr))
    {
    }

    // allocate memory for count mixin_data_in_object instances
    virtual char* alloc_mixin_data(size_t, const object*) override
    {
        CHECK(false);
        return nullptr;
    }

    virtual void dealloc_mixin_data(char*, size_t, const object*) override
    {
        CHECK(false);
    }

    virtual std::pair<char*, size_t> alloc_mixin(const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(&info == &m_info);
        return super::alloc_mixin(info, obj);
    }

    virtual void dealloc_mixin(char* ptr, size_t offset, const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(&info == &m_info);
        super::dealloc_mixin(ptr, offset, info, obj);
    }

    const basic_mixin_type_info& m_info;
};

class custom_alloc_2 : public custom_allocator<custom_alloc_2> {};
class custom_alloc_var : public custom_allocator<custom_alloc_var> {} the_allocator;

TEST_CASE("allocators")
{
    global_alloc* glob = new global_alloc;
    set_global_allocator(glob);

    {
        object o;
    }
    CHECK(alloc_counter<global_alloc>::data_allocations == 0);
    CHECK(alloc_counter<global_alloc>::mixin_allocations == 0);
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 0);
    CHECK(alloc_counter<custom_alloc_2>::mixin_allocations == 0);
    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 0);

    {
        object o;
        the_object = &o;
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
        the_object = &o1;
        mutate(o1)
            .add<normal_a>()
            .add<custom_1>()
            .add<custom_2_a>();

        object o2;
        the_object = &o2;
        mutate(o2)
            .add<normal_a>()
            .add<custom_1>()
            .add<custom_2_a>();

        object o3;
        the_object = &o3;
        mutate(o3)
            .add<normal_b>()
            .add<custom_2_b>();

        the_object = &o1;
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

        the_object = nullptr;
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

DYNAMIX_CONST_MESSAGE_0(int, get_i);
DYNAMIX_MESSAGE_0(int, get_x);

class custom_1 {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_1"; }
#endif

    int get_i() const
    {
        return i;
    }

    int i = 7;
};

class custom_own_var {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "normal_own_var"; }
#endif

    int get_x()
    {
        return x;
    }

    int x = 53;
};

#if DYNAMIX_OBJECT_REPLACE_MIXIN
TEST_CASE("mixin replacement")
{
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == alloc_counter<custom_alloc_1>::mixin_deallocations);
    alloc_counter<custom_alloc_1>::mixin_allocations = 0;
    alloc_counter<custom_alloc_1>::mixin_deallocations = 0;
    alloc_counter<custom_alloc_var>::mixin_allocations = 0;
    alloc_counter<custom_alloc_var>::mixin_deallocations = 0;

    object o;
    the_object = &o;

    mutate(o)
        .add<normal_a>()
        .add<normal_b>()
        .add<custom_1>()
        .add<custom_own_var>();

    auto& a_custom_1 = allocator<custom_alloc_1>();

    auto c1 = o.get<custom_1>();
    CHECK(c1->i == 7);
    c1->i = 123;
    auto& c1_info = _dynamix_get_mixin_type_info((custom_1*)nullptr);

    char* c1_new_buf;
    size_t c1_new_offset;
    std::tie(c1_new_buf, c1_new_offset) = a_custom_1.alloc_mixin(c1_info, &o);

    char* c1_old_buf;
    size_t c1_old_offset;

    std::tie(c1_old_buf, c1_old_offset) = o.move_mixin(DYNAMIX_MAX_MIXINS, c1_new_buf, c1_new_offset);
    CHECK(!c1_old_buf);
    CHECK(c1_old_offset == 0);

    auto& c2a_info = _dynamix_get_mixin_type_info((custom_2_a*)nullptr);
    std::tie(c1_old_buf, c1_old_offset) = o.move_mixin(c2a_info.id, c1_new_buf, c1_new_offset);
    CHECK(!c1_old_buf);
    CHECK(c1_old_offset == 0);

    std::tie(c1_old_buf, c1_old_offset) = o.move_mixin(c1_info.id, c1_new_buf, c1_new_offset);

    CHECK(c1_old_offset == c1_new_offset);
    // casting to intptr_t otherwise doctest will compare strings
    CHECK(reinterpret_cast<intptr_t>(c1_old_buf) != reinterpret_cast<intptr_t>(c1_new_buf));

    CHECK(reinterpret_cast<intptr_t>(c1_old_buf + c1_old_offset) == reinterpret_cast<intptr_t>(c1));

    auto c1_new = o.get<custom_1>();
    CHECK(c1_new != c1);
    CHECK(reinterpret_cast<intptr_t>(c1_new_buf + c1_new_offset) == reinterpret_cast<intptr_t>(c1_new));
    CHECK(c1_new->i == 123);
    CHECK(object_of(c1_new) == &o);

    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 2);
    // mixin must not be deallocated
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 0);

    a_custom_1.dealloc_mixin(c1_old_buf, c1_old_offset, c1_info, &o);

    CHECK(get_i(o) == 123);

    auto cov = o.get<custom_own_var>();
    cov->x = 401;
    auto& cov_info = _dynamix_get_mixin_type_info((custom_own_var*)nullptr);
    char* cov_new_buf;
    size_t cov_new_offset;
    std::tie(cov_new_buf, cov_new_offset) = the_allocator.alloc_mixin(cov_info, &o);

    memset(cov_new_buf, 0, sizeof(object*));

    new (cov_new_buf + cov_new_offset) custom_own_var();

    char* cov_old_buf;
    size_t cov_old_offset;
    std::tie(cov_old_buf, cov_old_offset) = o.hard_replace_mixin(cov_info.id, cov_new_buf, cov_new_offset);

    CHECK(cov_old_offset == cov_new_offset);
    // casting to intptr_t otherwise doctest will compare strings
    CHECK(reinterpret_cast<intptr_t>(cov_old_buf) != reinterpret_cast<intptr_t>(cov_new_buf));

    CHECK(reinterpret_cast<intptr_t>(cov_old_buf + cov_old_offset) == reinterpret_cast<intptr_t>(cov));

    auto cov_new = o.get<custom_own_var>();
    CHECK(cov_new != cov);
    CHECK(reinterpret_cast<intptr_t>(cov_new_buf + cov_new_offset) == reinterpret_cast<intptr_t>(cov_new));
    CHECK(cov_new->x == 53);
    CHECK((object_of(cov_new) == nullptr));

    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 2);
    // mixin must not be deallocated
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 0);

    the_allocator.dealloc_mixin(cov_old_buf, cov_old_offset, cov_info, &o);

    CHECK(get_x(o) == 53);

    o.reallocate_mixins();

    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 3);
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 2);
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 3);
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 2);

    CHECK(get_x(o) == 53);
    CHECK(get_i(o) == 123);
    CHECK(object_of(o.get<normal_a>()) == &o);
    CHECK(object_of(o.get<normal_b>()) == &o);
    CHECK(object_of(o.get<custom_1>()) == &o);
    CHECK(object_of(o.get<custom_own_var>()) == &o);

    mutate(o)
        .add<custom_2_a>();

#if DYNAMIX_USE_EXCEPTIONS
    CHECK_THROWS_AS(o.move_mixin(c2a_info.id, nullptr, 0), bad_mixin_move);
#endif

    the_object = nullptr;
}
#endif

class object_allocator_a : public object_allocator
{
public:
    static size_t data_allocations;
    static size_t data_deallocations;
    static size_t mixin_allocations;
    static size_t mixin_deallocations;

    // allocate memory for count mixin_data_in_object instances
    virtual char* alloc_mixin_data(size_t count, const object* obj) override
    {
        CHECK(obj == the_object);
        ++data_allocations;
        return _dda.alloc_mixin_data(count, obj);
    }

    virtual void dealloc_mixin_data(char* ptr, size_t count, const object* obj) override
    {
        CHECK((obj == the_object));
        ++data_deallocations;
        _dda.dealloc_mixin_data(ptr, count, obj);
    }

    virtual std::pair<char*, size_t> alloc_mixin(const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(obj == the_object);
        ++mixin_allocations;
        return _dda.alloc_mixin(info, obj);
    }

    virtual void dealloc_mixin(char* ptr, size_t offset, const basic_mixin_type_info& info, const object* obj) override
    {
        CHECK(offset == mixin_offset(ptr, info.alignment));
        CHECK((obj == the_object));
        ++mixin_deallocations;
        _dda.dealloc_mixin(ptr, offset, info, obj);
    }

    internal::default_allocator _dda;
};

size_t object_allocator_a::data_allocations;
size_t object_allocator_a::data_deallocations;
size_t object_allocator_a::mixin_allocations;
size_t object_allocator_a::mixin_deallocations;

class object_allocator_b : public object_allocator_a
{
public:
    ~object_allocator_b()
    {
        CHECK(ref_count == 0);
        CHECK(objects.empty());
    }

    virtual void on_set_to_object(object& owner) override
    {
        ++ref_count;
        CHECK(objects.find(&owner) == objects.end());
        objects.insert(&owner);
        CHECK(objects.size() == ref_count);
    }

    virtual void release(object& owner) noexcept override
    {
        CHECK(objects.find(&owner) != objects.end());
        objects.erase(&owner);
        --ref_count;
        CHECK(objects.size() == ref_count);
    }

    virtual object_allocator* on_copy_construct(object& target, const object& source) override
    {
        CHECK(objects.find(&source) != objects.end());
        CHECK(source.allocator() == this);
        return this;
    }

    virtual object_allocator* on_move(object& target, object& source) noexcept override
    {
        CHECK(source.allocator() == this);
        auto f = objects.find(&source);
        CHECK(f != objects.end());
        objects.erase(f);
        CHECK(objects.find(&target) == objects.end());
        --ref_count;
        return this;
    }

    std::set<const object*> objects;
    size_t ref_count = 0;
};

TEST_CASE("object allocator")
{
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == alloc_counter<custom_alloc_1>::mixin_deallocations);
    alloc_counter<custom_alloc_1>::mixin_allocations = 0;
    alloc_counter<custom_alloc_1>::mixin_deallocations = 0;
    alloc_counter<custom_alloc_var>::mixin_allocations = 0;
    alloc_counter<custom_alloc_var>::mixin_deallocations = 0;

    {
        object_allocator_a alloc;
        object o(&alloc);
        CHECK(o.allocator() == &alloc);
    }

    CHECK(object_allocator_a::data_allocations == 0);
    CHECK(object_allocator_a::mixin_allocations == 0);

    {
        object_allocator_a alloc;
        object o(&alloc);
        the_object = &o;

        mutate(o)
            .add<normal_a>()
            .add<custom_1>()
            .add<custom_own_var>();

        CHECK(get_i(o) == 7);
        CHECK(get_x(o) == 53);

        mutate(o)
            .remove<normal_a>()
            .add<normal_b>();
    }

    CHECK(object_allocator_a::data_allocations == 2);
    CHECK(object_allocator_a::data_deallocations == 2);
    CHECK(object_allocator_a::mixin_allocations == 4);
    CHECK(object_allocator_a::mixin_deallocations == 4);
    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 0);
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 0);
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 0);
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 0);

    {
        object_allocator_a alloc;
        object o1(&alloc);
        the_object = &o1;

        mutate(o1)
            .add<normal_a>()
            .add<normal_b>()
            .add<custom_1>()
            .add<custom_own_var>();

        object o2 = std::move(o1);
        the_object = &o2;

        CHECK(o1.empty());
        CHECK(!o1.allocator());

        CHECK(o2.allocator() == &alloc);

        {
            object o3;
            the_object = &o3;

            o3.copy_from(o2);
            CHECK(!o3.allocator());
            CHECK(o2.allocator() == &alloc);
        }

        the_object = &o2;
    }

    CHECK(object_allocator_a::data_allocations == 3);
    CHECK(object_allocator_a::data_deallocations == 3);
    CHECK(object_allocator_a::mixin_allocations == 8);
    CHECK(object_allocator_a::mixin_deallocations == 8);
    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 1);
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 1);
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 1);
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 1);

    object_allocator_b alloc_b;
    {
        object o1(&alloc_b);
        the_object = &o1;

        mutate(o1)
            .add<normal_a>()
            .add<normal_b>()
            .add<custom_1>()
            .add<custom_own_var>();

        object o2 = std::move(o1);
        the_object = &o2;

        CHECK(o1.empty());
        CHECK(!o1.allocator());

        CHECK(o2.allocator() == &alloc_b);

        {
            object o3;
            the_object = &o3;

            o3.copy_from(o2);
            CHECK(o3.allocator() == &alloc_b);
            CHECK(o2.allocator() == &alloc_b);
        }

        object o4(&alloc_b);

        the_object = &o2;
    }
    CHECK(alloc_b.ref_count == 0);
    CHECK(alloc_b.objects.empty());

    CHECK(alloc_counter<custom_alloc_var>::mixin_allocations == 1);
    CHECK(alloc_counter<custom_alloc_var>::mixin_deallocations == 1);
    CHECK(alloc_counter<custom_alloc_1>::mixin_allocations == 1);
    CHECK(alloc_counter<custom_alloc_1>::mixin_deallocations == 1);
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
class custom_2_a {
public:
    custom_2_a() = default;
    custom_2_a(custom_2_a&&) = delete;

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

DYNAMIX_DEFINE_MIXIN(normal_a, dynamix::none);
DYNAMIX_DEFINE_MIXIN(normal_b, dynamix::none);
DYNAMIX_DEFINE_MIXIN(custom_1, dynamix::allocator<custom_alloc_1>() & get_i_msg);
DYNAMIX_DEFINE_MIXIN(custom_2_a, dynamix::allocator<custom_alloc_2>());
DYNAMIX_DEFINE_MIXIN(custom_2_b, dynamix::allocator<custom_alloc_2>());
DYNAMIX_DEFINE_MIXIN(custom_own_var, the_allocator & get_x_msg);

DYNAMIX_DEFINE_MESSAGE(get_i);
DYNAMIX_DEFINE_MESSAGE(get_x);
