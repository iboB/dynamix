// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once


/**
 * \file
 * Defines message related operations for the feature list.
 */


#include "feature.hpp"

namespace dynamix
{

namespace internal
{

struct DYNAMIX_API message_for_mixin;

// used for a general function address for all message calls
typedef void(*func_ptr)();

// feature tags are used by feature parsers and the domain to distinguish between feature types
// and register them appropriately
struct DYNAMIX_API message_feature_tag {};

struct DYNAMIX_API message_t : public feature
{
    enum e_mechanism
    {
        unicast,
        multicast,

        num_mechanism_types
    };

    const e_mechanism mechanism;

    /* the way messages identify themselves to feature parsers and the domain */
    typedef message_feature_tag feature_tag;

    // default message implementation (if any)
    message_for_mixin* default_impl_data;

protected:
    message_t(const char* name, e_mechanism mecha, bool is_private)
        : feature(name, is_private)
        , mechanism(mecha)
        , default_impl_data(nullptr)
    {}
};

template <typename Message>
struct message_priority
{
    int priority;
};

// a structure that describes a message with specific data for a concrete mixin
struct DYNAMIX_API message_for_mixin
{
    message_t* message; // message object
    mixin_id _mixin_id; // id of the handling mixin type information

    // the caller member is a pointer to a template function instantiated by the message macros
    // this function takes the appropriate parameters as arguments and is instantiated to call
    // the mixin method, even from void*
    // code based on
    // http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
    func_ptr caller;

    // message perks
    int priority;
};


// metafunction used to register default implementations of messages
// in case no mixin implements them (if it does, it will also register the message)
template <typename Message>
struct message_default_impl_registrator
{
    message_default_impl_registrator()
    {
        _dynamix_register_mixin_feature((Message*)nullptr);
    }

    // "payload" instance of the type
    static message_default_impl_registrator registrator;

    // as with mixin_type_info_instance, this is
    // to prevent warnings and optimizations that will say that we're not using
    // instantiator by simply referencing it
    int unused;
};
template <typename Message>
message_default_impl_registrator<Message> message_default_impl_registrator<Message>::registrator;


} // namespace internal

// Used in the mixin's feature list to give a priority to a message.
template <typename Message>
internal::message_priority<Message> priority(int p, Message*)
{
    internal::message_priority<Message> mp;
    mp.priority = p;
    return mp;
}

} // namespace dynamix
