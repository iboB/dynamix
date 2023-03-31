// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/msg/next_impl.hpp>

namespace dynamix::v1compat {
template <typename Message, typename Mixin>
bool has_next_bidder(Message*, Mixin* mixin) {
    return has_next_bidder_set<Message>(mixin);
}

template <typename Message, typename Mixin, typename... Args>
auto call_next_bidder(Message*, Mixin* mixin, Args&&... args) -> typename msg_traits<Message>::ret_t {
    if constexpr (Message::multicast) {
        return call_next_bidder_set<Message>(mixin, std::forward<Args>(args)...);
    }
    else {
        return call_next_bidder_top<Message>(mixin, std::forward<Args>(args)...);
    }
}
}

#define DYNAMIX_V1_HAS_NEXT_BIDDER(message_tag) \
    ::dynamix::v1compat::has_next_bidder(message_tag, this)

#define DYNAMIX_V1_CALL_NEXT_BIDDER(message_tag, ...) \
    ::dynamix::v1compat::call_next_bidder(message_tag, this, ##__VA_ARGS__)
