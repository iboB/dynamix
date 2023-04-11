// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "domain.hpp"
#include <dynamix/define_mixin.hpp>

namespace dynamix::v1compat {

struct none_t {};
inline none_t none;

struct user_data {
    user_data(uintptr_t d) noexcept : data(d) {}
    uintptr_t data;
};

struct bound_impl {
    const feature_info& info;
    void* payload;
};

template <typename Message, typename Func>
bound_impl bind(Message*, Func f) {
    typename Message::traits::func_t fc = f;
    return {
        g::get_feature_info_safe<Message>(),
        reinterpret_cast<feature_payload>(fc)
    };
}

struct mixin_name {
    std::string_view name;
    mixin_name(std::string_view n) : name(n) {}
};

// templated so the type can be passed along with the perks
template <typename Message>
struct message_perks
{
    int32_t bid = 0;
    int32_t priority = 0;
};

template <typename Message>
message_perks<Message> upriority(int p, Message*) {
    message_perks<Message> mp;
    mp.priority = p;
    return mp;
}

template <typename Message>
message_perks<Message> priority(int p, Message* msg)
{
    if constexpr (!Message::multicast) {
        [[maybe_unused]] [[deprecated("unicast priority is inverted")]] int x = 0;
    }
    return upriority(p, msg);
}

template <typename Message>
message_perks<Message> bid(int b, Message*)
{
    message_perks<Message> mp;
    mp.bid = b;
    return mp;
}

template <typename Message, template <typename> class Perks>
Perks<Message> upriority(int p, Perks<Message> perks)
{
    perks.priority = p;
    return perks;
}

template <typename Message, template <typename> class Perks>
Perks<Message> priority(int p, Perks<Message> perks)
{
    if constexpr (!Message::multicast) {
        [[maybe_unused]] [[deprecated("unicast priority is inverted")]] int x = 0;
    }
    return upriority(p, perks);
}

template <typename Message, template <typename> class Perks>
Perks<Message> bid(int b, Perks<Message> perks)
{
    perks.bid = b;
    return perks;
}

template <typename Mixin>
class feature_parser : public util::mixin_info_data_builder<Mixin> {
    using super = util::mixin_info_data_builder<Mixin>;
public:
    feature_parser(util::mixin_info_data& data, dnmx_sv name) noexcept
        : super(data, name)
    {
        data.info.force_external = true;
    }

    template <typename Message>
    feature_parser& operator&(Message*) {
        super::template implements<Message>();
        return *this;
    }

    feature_parser& operator&(none_t) {
        return *this;
    }

    feature_parser& operator&(user_data d) {
        super::user_data(d.data);
        return *this;
    }

    feature_parser& operator&(bound_impl b) {
        super::implements_by(b.info, b.payload);
        return *this;
    }

    feature_parser& operator&(mixin_name n) {
        super::name(n.name);
        return *this;
    }

    template <typename Message>
    feature_parser& operator&(message_perks<Message> mp)
    {
        super::template implements<Message>(util::builder_literals::feature_bid_priority(mp.bid, mp.priority));
        return *this;
    }
};
}

#define DYNAMIX_V1_DEFINE_MIXIN(mixin, features) \
    DYNAMIX_DEFINE_MIXIN_EX(::dynamix::v1compat::domain_tag, mixin, ::dynamix::v1compat::feature_parser) & features
