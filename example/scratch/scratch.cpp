// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <iostream>

DYNAMIX_DECLARE_MIXIN(ma);
DYNAMIX_DECLARE_MIXIN(mb);

// the object template argument is actually dynamix object BUT with the msg constness added to it
template <typename Derived, typename Object, typename Ret, typename... Args>
struct msg_caller : public ::dynamix::internal::message_t
{
    msg_caller(const char* message_name, dynamix::internal::message_t::e_mechanism mechanism)
        : ::dynamix::internal::message_t(message_name, mechanism, false)
    {}

    using caller_func = Ret(*)(void*, Args...);

    // makes the actual method call
    template <typename Mixin, Ret(Mixin::*Method)(Args...)>
    static Ret caller(void* mixin, Args... args)
    {
        auto m = reinterpret_cast<Mixin*>(mixin);
        return (m->*Method)(std::forward<Args>(args)...);
    }

    // calls for different types of mechanisms 

    // unicast
    static Ret uni(Object& obj, Args&&... args)
    {
        const ::dynamix::feature& self = _dynamix_get_mixin_feature_fast(static_cast<Derived*>(nullptr));
        DYNAMIX_ASSERT(static_cast<const ::dynamix::internal::message_t&>(self).mechanism
            == ::dynamix::internal::message_t::unicast);

        const dynamix::internal::object_type_info::call_table_entry& call_entry =
            obj._type_info->_call_table[self.id];

        const ::dynamix::internal::message_for_mixin* msg_data = call_entry.top_bid_message;
        DYNAMIX_MSG_THROW_UNLESS(msg_data, ::dynamix::bad_message_call);

        // unfortunately we can't assert(_d_msg_data->message == &_d_self); since the data might come from a different module

        char* mixin_data =
            // skipping several function calls, which greatly improves build time
            reinterpret_cast<char*>(const_cast<void*>(obj._mixin_data[obj._type_info->_mixin_indices[self.id]].mixin()));

        auto func = reinterpret_cast<caller_func>(msg_data->caller);

        return func(mixin_data, std::forward<Args>(args)...);
    }
};

struct dynamix_msg_add2 : public
    msg_caller<dynamix_msg_add2, dynamix::object, int, int>
{
    dynamix_msg_add2()
        : msg_caller("add2", dynamix::internal::message_t::unicast)
    {}

    template <typename Mixin>
    ::dynamix::internal::func_ptr get_caller_for() const
    {
        // prevent the linker from optimizing away the caller function
        static caller_func the_caller = caller<Mixin, &Mixin::add2>;
        // cast the caller to a void (*)() - safe according to the standard
        return reinterpret_cast<::dynamix::internal::func_ptr>(the_caller);
    }
};

int add2(dynamix::object& object, int n)
{
    return dynamix_msg_add2::uni(object, std::forward<int>(n));
}

dynamix_msg_add2* add2_msg;

::dynamix::feature& _dynamix_get_mixin_feature_safe(const dynamix_msg_add2*)
{
    return ::dynamix::internal::feature_instance<dynamix_msg_add2>::the_feature_safe();
}

const ::dynamix::feature& _dynamix_get_mixin_feature_fast(const dynamix_msg_add2*)
{
    return ::dynamix::internal::feature_instance<dynamix_msg_add2>::the_feature_fast();
}

void _dynamix_register_mixin_feature(const dynamix_msg_add2*)
{
    ::dynamix::internal::domain::safe_instance().
        register_feature(::dynamix::internal::feature_instance<dynamix_msg_add2>::the_feature_safe());
}

DYNAMIX_MULTICAST_MESSAGE_1(int, add1, int, n);

int main()
{
    using namespace dynamix;
    using namespace std;

    object o;

    mutate(o)
        .add<ma>()
        .add<mb>();

    cout << add2(o, 5) << endl;
    cout << add1<combinators::sum>(o, 3) << endl;

    return 0;
}


class ma
{
public:
    int add2(int n)
    {
        return n + 2;
    }

    int add1(int n)
    {
        return n + 1;
    }
};

class mb
{
public:
    int add1(int n)
    {
        return n - 1; // haha
    }
};

DYNAMIX_DEFINE_MESSAGE(add1);

DYNAMIX_DEFINE_MIXIN(ma, add2_msg & add1_msg);
DYNAMIX_DEFINE_MIXIN(mb, add1_msg);
