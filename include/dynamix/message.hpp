// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
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
#include <type_traits>

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

// templated so the type can be passed along with the perks
template <typename Message>
struct message_perks
{
    int bid = 0;
    int priority = 0;
};

// a structure that describes a message with specific data for a concrete mixin
struct DYNAMIX_API message_for_mixin
{
    message_t* message; // message object

    // the caller member is a pointer to a lambda created by the message macros
	// (see get_caller_for)
    // this function takes the appropriate parameters as arguments and calls
    // the mixin method from void*
    func_ptr caller;

    // message perks
    int bid;
    int priority;
};


// metafunction used to register messages
// in case no mixin implements them (if it does, it will also register the message)
template <typename Message>
struct message_registrator
{
    message_registrator()
    {
        _dynamix_register_mixin_feature(static_cast<Message*>(nullptr));
    }

    // defined in message_macros because it depends on domain.hpp
    // unregisteres the message
    ~message_registrator();

    // "payload" instance of the type
    static message_registrator registrator;

    // as with mixin_type_info_instance, this is
    // to prevent warnings and optimizations that will say that we're not using
    // instantiator by simply referencing it
    int unused;
};
template <typename Message>
message_registrator<Message> message_registrator<Message>::registrator;

// check if a class has a method set_num_results
template <typename Combinator>
struct has_set_num_results
{
private:
    template<typename C> static auto test(int) -> decltype(std::declval<C>().set_num_results(1), std::true_type());
    template<typename> static std::false_type test(...);
public:
    static constexpr bool value = std::is_same<decltype(test<Combinator>(0)), std::true_type>::value;
};

// call set_num_results for combinators that have it
template <typename Combinator>
typename std::enable_if<has_set_num_results<Combinator>::value>::type
set_num_results_for(Combinator& c, size_t num_results)
{
    c.set_num_results(num_results);
}

// do nothing for combinators that don't
template <typename Combinator>
typename std::enable_if<!has_set_num_results<Combinator>::value>::type
set_num_results_for(Combinator&, size_t) {}

// other message wrappers

struct message_wrapper {};

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

// Perks of message wrappers

// So perks can be passed as arguments to one another
template <typename Feature>
internal::message_perks<Feature> priority(int p, internal::message_perks<Feature> perks)
{
    perks.priority = p;
    return perks;
}

template <typename Feature>
internal::message_perks<Feature> bid(int b, internal::message_perks<Feature> perks)
{
    perks.bid = b;
    return perks;
}

} // namespace dynamix
