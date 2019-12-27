// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Defines the internal class used to parse a mixin's features.
 */


#include "../config.hpp"

#include "../features.hpp"
#include "../object_type_info.hpp"
#include "../message_features.hpp"

namespace dynamix
{

class mixin_allocator;

namespace internal
{

// in the phase1 parser we
// 1. set unique features if they are provided
// 2. register multi features
// 2. count multi features so buffers for them can be allocated in phase 2
class DYNAMIX_API feature_parser_phase_1
{
public:
    feature_parser_phase_1(mixin_type_info& info) : info(info) {}

    template <typename Feature>
    feature_parser_phase_1& operator & (const Feature*)
    {
        _dynamix_register_mixin_feature(static_cast<Feature*>(nullptr));
        count_feature(typename Feature::feature_tag());
        return *this;
    }

    template <typename Message>
    feature_parser_phase_1& operator & (const message_perks<Message>&)
    {
        return operator&(static_cast<Message*>(nullptr));
    }

    feature_parser_phase_1& operator & (mixin_allocator& allocator)
    {
        info.allocator = &allocator;
        return *this;
    }

    feature_parser_phase_1& operator & (mixin_name_feature n)
    {
        info.name = n.name;
        return *this;
    }

    feature_parser_phase_1& operator & (const noop_feature_t*) { return *this; }

    // counters
    size_t num_messages() const { return _num_messages; }
private:

    void count_feature(const message_feature_tag&)
    {
        ++_num_messages;
    }

    mixin_type_info& info;
    size_t _num_messages = 0;
};


// in phase 2 we
// parse mixin multi features and apply them to the mixin type info
template <typename Mixin>
class feature_parser_phase_2
{
public:
    feature_parser_phase_2(mixin_type_info& info) : info(info) {}

    template <typename Feature>
    feature_parser_phase_2& operator & (const Feature*)
    {
        Feature& f = get_registered_feature<Feature>();
        parse_feature(f, typename Feature::feature_tag());
        return *this;
    }

    template <typename Message>
    feature_parser_phase_2& operator & (message_perks<Message> mp)
    {
        Message& msg = get_registered_feature<Message>();
        parse_message(msg, mp.bid, mp.priority, msg.template get_caller_for<Mixin>());
        return *this;
    }

    template <typename Message>
    feature_parser_phase_2& operator & (message_perks_and_caller<Message> mp)
    {
        Message& msg = get_registered_feature<Message>();
        parse_message(msg, mp.bid, mp.priority, mp.caller);
        return *this;
    }

    // unique_features which we con't care about at this phase
    feature_parser_phase_2& operator & (mixin_allocator&) { return *this; }
    feature_parser_phase_2& operator & (mixin_name_feature) { return *this; }
    feature_parser_phase_2& operator & (const noop_feature_t*) { return *this; }

private:
    template <typename Feature>
    Feature& get_registered_feature()
    {
        Feature& f = static_cast<Feature&>(_dynamix_get_mixin_feature_safe(static_cast<Feature*>(nullptr)));
        I_DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID); // must be registered
        return f;
    }

    template <typename Message>
    void parse_feature(Message& msg, const message_feature_tag&)
    {
        parse_message(msg, 0, 0, msg.template get_caller_for<Mixin>());
    }

    void parse_message(message_t& msg, int bid, int priority, func_ptr caller)
    {
#if DYNAMIX_DEBUG
        // check for duplicate entries
        for (const message_for_mixin& msg_info : info.message_infos)
        {
            I_DYNAMIX_ASSERT(msg_info.message); // null message ???
            I_DYNAMIX_ASSERT(msg_info.message != &msg); // duplicate message. You have "x_msg & ... & x_msg"
        }
#endif
        info.message_infos.emplace_back();
        message_for_mixin& mfm = info.message_infos.back();
        mfm.message = &msg;
        mfm.caller = caller;
        mfm.bid = bid;
        mfm.priority = priority;
    }

    mixin_type_info& info;
};

} // namespace internal
} // namespace dynamix
