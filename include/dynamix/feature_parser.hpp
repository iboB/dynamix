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
 * Defines the internal class used to parse a mixin's features.
 */


#include "global.hpp"
#include "message.hpp"
#include "features.hpp"
#include "object_type_info.hpp"
#include "allocators.hpp" // clang needs this to assign mixin_allocator to allocator :(

namespace dynamix
{

class mixin_allocator;

namespace internal
{

// see comments in feature_instance on why this class is needed
class DYNAMIX_API feature_registrator
{
public:
    template <typename Feature>
    feature_registrator& operator & (const Feature*)
    {
        _dynamix_register_mixin_feature(static_cast<Feature*>(nullptr));
        return *this;
    }

    template <typename Message>
    feature_registrator& operator & (message_perks<Message>)
    {
        return operator&(static_cast<Message*>(nullptr));
    }

    feature_registrator& operator & (mixin_allocator&)
    {
        // nothing special to do here
        return *this;
    }

    feature_registrator& operator & (mixin_name_feature n)
    {
        mixin_name = n.name;
        return *this;
    }

    feature_registrator& operator & (const noop_feature_t*)
    {
        return *this;
    }

    void operator & (const no_features_t*)
    {
    }

    const char* mixin_name = nullptr;
};


// parse mixin features and apply them to the mixin type info
template <typename Mixin>
class feature_parser
{
public:
    template <typename Feature>
    feature_parser& operator & (const Feature*)
    {
        Feature& f = static_cast<Feature&>(_dynamix_get_mixin_feature_safe(static_cast<Feature*>(nullptr)));
        DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);

        mixin_type_info& mixin_info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        parse_feature(mixin_info, f, typename Feature::feature_tag());

        return *this;
    }

    template <typename Message>
    feature_parser& operator & (message_perks<Message> mp)
    {
        parse_message_with_perks<Message>(mp.bid, mp.priority);
        return *this;
    }

    feature_parser& operator & (mixin_allocator& allocator)
    {
        mixin_type_info& mixin_info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        mixin_info.allocator = &allocator;

        return *this;
    }

    void operator & (const no_features_t*)
    {
    }

    feature_parser& operator & (mixin_name_feature)
    {
        return *this;
    }

    feature_parser& operator & (const noop_feature_t*)
    {
        return *this;
    }

private:
    template <typename Message>
    void parse_feature(mixin_type_info& mixin_info, Message& msg, const message_feature_tag&)
    {
        parse_message(mixin_info, msg, 0, 0);
    }

    template <typename Message>
    void parse_message_with_perks(int bid, int priority)
    {
        Message& msg = static_cast<Message&>(_dynamix_get_mixin_feature_safe(static_cast<Message*>(nullptr)));
        DYNAMIX_ASSERT(msg.id != INVALID_FEATURE_ID);

        mixin_type_info& mixin_info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        parse_message(mixin_info, msg, bid, priority);
    }

    template <typename Message>
    void parse_message(mixin_type_info& mixin_info, Message& msg, int bid, int priority)
    {
#if DYNAMIX_DEBUG
        // check for duplicate entries
        for (const message_for_mixin& msg_info : mixin_info.message_infos)
        {
            DYNAMIX_ASSERT(msg_info.message); // null message ???
            DYNAMIX_ASSERT(msg_info.message != &msg); // duplicate message. You have "x_msg & ... & x_msg"
        }
#endif
        mixin_info.message_infos.resize(mixin_info.message_infos.size() + 1);
        message_for_mixin& mfm = mixin_info.message_infos.back();
        mfm.message = &msg;
        mfm._mixin_id = mixin_info.id;
        mfm.caller = msg.template get_caller_for<Mixin>();
        mfm.bid = bid;
        mfm.priority = priority;
    }
};

} // namespace internal
} // namespace dynamix
