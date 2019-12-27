// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "message.hpp"

namespace dynamix
{

namespace internal
{

// templated so the type can be passed along with the perks
template <typename Message>
struct message_perks
{
    int bid = 0;
    int priority = 0;
};

// used for custom callers
template <typename Message>
struct message_perks_and_caller : public message_perks<Message>
{
    func_ptr caller = nullptr;
};

} // namespace internal

// Used in the mixin's feature list to set perks to messages
template <typename Message>
internal::message_perks<Message> priority(int p, Message*)
{
    internal::message_perks<Message> mp;
    mp.priority = p;
    return mp;
}

template <typename Message>
internal::message_perks<Message> bid(int b, Message*)
{
    internal::message_perks<Message> mp;
    mp.bid = b;
    return mp;
}

// bind a `void* self` function to the message
template <typename Message>
internal::message_perks_and_caller<Message> bind(Message*, typename Message::caller_func caller)
{
    internal::message_perks_and_caller<Message> mp;
    mp.caller = reinterpret_cast<internal::func_ptr>(caller);
    return mp;
}

// if we ever want type safety to the bound functions we need to use this implementation:
//
// template <typename Message, typename Caller>
// internal::message_perks_and_caller<Message> bind(Message*, Caller c)
// {
//     internal::message_perks_and_caller<Message> mp;
//     mp.caller = Message::bind_caller(c);
//     return mp;
// }
//
// which also requires all message structs to have an additional function bind_caller (besides get_caller_for)
/*
        template <typename Mixin> \
        static ::dynamix::internal::func_ptr bind_caller(return_type (*caller)(constness Mixin*, arg0_type)) \
        { \
            static return_type (*bound_caller)(constness Mixin*, arg0_type) = caller; \
            static caller_func the_caller = [](void* _d_mixin, arg0_type a0) -> return_type \
            { \
                constness Mixin* _d_m = reinterpret_cast<Mixin*>(_d_mixin); \
                return bound_caller(_d_m, std::forward<arg0_type>(a0)); \
            }; \
            return reinterpret_cast< ::dynamix::internal::func_ptr>(the_caller); \
        } \
*/
// we could also implement the function from above with a template caller and allow the bound function to be
// a std::function or perform implicit casts of arguments
//
// if we want to be really clever at the cost of some undefined behavior ... :)
// we could take `Mixin* self` or `Mixin& self` and just reinterpret_cast the heck out of this to func_ptr
// relying on the fact that no sane compiler implements fat pointers for structs and classes
// for now though void* bind should suffice

// Perks of message wrappers

// So perks can be passed as arguments to one another
template <typename Message, template <typename> class Perks>
Perks<Message> priority(int p, Perks<Message> perks)
{
    perks.priority = p;
    return perks;
}

template <typename Message, template <typename> class Perks>
Perks<Message> bid(int b, Perks<Message> perks)
{
    perks.bid = b;
    return perks;
}

} // namespace dynamix
