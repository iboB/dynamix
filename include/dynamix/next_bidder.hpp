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
* Functions and macros associated with calling and querying the next bidder of
* a message,
*/

#include "config.hpp"
#include "message_macros.hpp"
#include "exception.hpp"
#include "object.hpp"

namespace dynamix
{

template <typename Mixin, typename Message, typename ...Args>
auto call_next_bidder(Mixin* mixin, Message* message, Args&&... args)
-> decltype(std::declval<typename Message::caller_func>()(nullptr, std::forward<Args>(args)...))
{
    const mixin_type_info& mixin_info = _dynamix_get_mixin_type_info(mixin);
    auto& msg = static_cast<const internal::message_t&>(_dynamix_get_mixin_feature_fast(message));
    const object* obj = object_of(mixin);
    const internal::object_type_info::call_table_entry& entry = obj->_type_info->_call_table[msg.id];
    const size_t mixin_index = obj->_type_info->_mixin_indices[mixin_info.id];

    DYNAMIX_MSG_THROW_UNLESS(entry.top_bid_message, bad_message_call);

    if (msg.mechanism == internal::message_t::unicast)
    {
        // find function after the current one
        auto ptr = entry.begin;
        DYNAMIX_THROW_UNLESS(ptr, bad_next_bidder_call); // no next bidder calls

        // since this is called from a mixin this loop must end eventually
        while (ptr++->mixin_index != mixin_index) I_DYNAMIX_ASSERT(ptr < entry.end);
        DYNAMIX_THROW_UNLESS(ptr < entry.end, bad_next_bidder_call);

        // for unicasts the next message for mixin (pointed by ptr) must be the one
        // we want to execute (with the next bid)
        auto data = reinterpret_cast<char*>(const_cast<void*>(obj->_mixin_data[ptr->mixin_index].mixin()));
        auto func = reinterpret_cast<typename Message::caller_func>(ptr->caller);
        return func(data, std::forward<Args>(args)...);
    }
    else
    {
        // find function after the current one
        auto ptr = entry.begin;
        // since this is called from a mixin this loop must end eventually
        // no way to assert anything sensible here
        while (ptr++->mixin_index != mixin_index);

        // buffer ends here, so there are no next bidders
        DYNAMIX_THROW_UNLESS(*ptr, bad_next_bidder_call);

        // for multicasts this might be the message we're looking for or simply the next
        // in the chain of priorities
        // so check the previous one's bid and if it's the same search for the next one that's different
        const internal::object_type_info::call_table_message* prev_msg = ptr - 1;

        // loop to the end of the bid chain
        while (ptr->data->bid == prev_msg->data->bid) ++ptr;

        I_DYNAMIX_ASSERT(ptr >= entry.end); // we must be past the end here

        // after the end of the bid chain we could STILL have reached the end of the buffer
        DYNAMIX_THROW_UNLESS(*ptr, bad_next_bidder_call);

        auto bid = ptr->data->bid;

        // execute the bid chain
        for (;;)
        {
            auto data = reinterpret_cast<char*>(const_cast<void*>(obj->_mixin_data[ptr->mixin_index].mixin()));
            auto func = reinterpret_cast<typename Message::caller_func>(ptr->caller);
            ++ptr;
            // check next message data
            if (!(*ptr) || ptr->data->bid != bid)
            {
                // end of bid chain
                // return last call
                return func(data, std::forward<Args>(args)...);
            }
            else
            {
                // just execute function
                func(data, std::forward<Args>(args)...);
            }
        }
    }
}

template <typename Mixin, typename Message>
bool has_next_bidder(Mixin* mixin, Message* message)
{
    const mixin_type_info& mixin_info = _dynamix_get_mixin_type_info(mixin);
    auto& msg = static_cast<const internal::message_t&>(_dynamix_get_mixin_feature_fast(message));
    const object* obj = object_of(mixin);
    const internal::object_type_info::call_table_entry& entry = obj->_type_info->_call_table[msg.id];
    const size_t mixin_index = obj->_type_info->_mixin_indices[mixin_info.id];

    if (!entry.top_bid_message) return false;

    if (msg.mechanism == internal::message_t::unicast)
    {
        auto ptr = entry.begin;
        if (!ptr) return false;
        while (ptr++->mixin_index != mixin_index) I_DYNAMIX_ASSERT(ptr < entry.end);
        return ptr < entry.end;
    }
    else
    {
        auto ptr = entry.begin;
        while (ptr++->mixin_index != mixin_index);
        if (!*ptr) return false;
        const internal::object_type_info::call_table_message* prev_msg = ptr - 1;
        while (ptr->data->bid == prev_msg->data->bid) ++ptr;
        return !!*ptr;
    }
}

} // namespace dynamix

/**
* \brief Macro that calls the next bidder from a message with a higher bid
*
* Use it in the method which implements a given message. The return type is the same
* as the message return type.
*
* \param message_tag is the message tag for the required message (ie `foo_msg`)
* \param args are the arguments for this message. Typically the same as the
*  arguments of the current method.
*/
#define DYNAMIX_CALL_NEXT_BIDDER(message_tag, ...) \
    ::dynamix::call_next_bidder(this, message_tag, ##__VA_ARGS__)

/**
* \brief Macro that checks if a bidder with a lower bid exists from a message
* with a higher bid
*
* Use it in the method which implements a given message.
*
* \param message_tag is the message tag for the required message (ie `foo_msg`)
*/
#define DYNAMIX_HAS_NEXT_BIDDER(message_tag) \
    ::dynamix::has_next_bidder(this, message_tag)
