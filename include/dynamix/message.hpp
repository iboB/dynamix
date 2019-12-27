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

} // namespace internal
} // namespace dynamix
