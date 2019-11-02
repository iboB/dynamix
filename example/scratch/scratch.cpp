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

class xxx : public b {};

DYNAMIX_DEFINE_MIXIN(b, testv_msg);

struct script_mixin
{
    int n = 10;
};

void script_testv(void* m, vector<int> data)
{
    auto sm = reinterpret_cast<script_mixin*>(m);
    cout << "script(" << sm->n << "): " << data.size() << endl;
}

namespace is_mixin_ns
{

template <typename T>
static int _dynamix_get_mixin_type_info(const T*); // better mach than global func for implicit casts

template <typename T>
struct is_mixin
{
    static constexpr bool value = !std::is_same<int, decltype(_dynamix_get_mixin_type_info(std::declval<T*>()))>::value;
};

}

using is_mixin_ns::is_mixin;

//template <typename T>
//using is_mixin = std::is_same<T, decltype(extract_arg((::dynamix::internal::mixin_type_info&(*)(const T*))_dynamix_get_mixin_type_info))>;

int main()
{
    cout << is_mixin<a>::value << endl;
    cout << is_mixin<b>::value << endl;
    cout << is_mixin<xxx>::value << endl;

    auto& dom = dynamix::internal::domain::safe_instance();
    dynamix::internal::mixin_type_info info0;

    info0.name = "bagavag";
    info0.size = sizeof(script_mixin);
    info0.alignment = std::alignment_of<script_mixin>::value;
    info0.constructor = &dynamix::internal::call_mixin_constructor<script_mixin>;
    info0.destructor = &dynamix::internal::call_mixin_destructor<script_mixin>;
    info0.copy_constructor = dynamix::internal::get_mixin_copy_constructor<script_mixin>();
    info0.copy_assignment = dynamix::internal::get_mixin_copy_assignment<script_mixin>();
    info0.move_constructor = dynamix::internal::get_mixin_move_constructor<script_mixin>();
    info0.allocator = dom._allocator;

    dom.internal_register_mixin_type(info0);

    info0.message_infos.emplace_back();
    auto& msg = info0.message_infos.back();
    msg.bid = msg.priority = 0;
    msg._mixin_id = info0.id;
    msg.caller = reinterpret_cast<dynamix::internal::func_ptr>(script_testv);
    msg.message = static_cast<dynamix::internal::message_t*>(&_dynamix_get_mixin_feature_safe(testv_msg));

    auto hero = new dynamix::object;
    dynamix::mutate(hero)
        .add<a>()
        .add<b>()
        .add("bagavag");

    vector<int> foo = {1, 2, 3};
    testv(hero, foo);

    return 0;
}

DYNAMIX_DEFINE_MESSAGE(testv);
