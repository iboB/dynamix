// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "internal/preprocessor.hpp"
#include "internal/message_macros.hpp"
#include "domain.hpp"

namespace dynamix
{
namespace internal
{
// type used to register messages in case no mixin implements them
// (all mixins which implement a message will also register it)
template <typename Message>
struct message_registrator
{
    message_registrator()
    {
        _dynamix_register_mixin_feature(static_cast<Message*>(nullptr));
    }

    // defined in message_macros because it depends on domain.hpp
    // unregisteres the message
    ~message_registrator()
    {
        internal::domain::safe_instance().
            unregister_feature(static_cast<message_t&>(_dynamix_get_mixin_feature_safe(static_cast<Message*>(nullptr))));
    }

    // "payload" instance of the type
    static message_registrator registrator;

    // as with mixin_type_info_instance, this is
    // to prevent warnings and optimizations that will say that we're not using
    // instantiator by simply referencing it
    int unused;
};
template <typename Message>
message_registrator<Message> message_registrator<Message>::registrator;

} // namespace internal
} // namespace dynamix

/**
* The macro for defining a message.
* Use it once per message in a compilation unit (.cpp file)
*/
#define DYNAMIX_DEFINE_MESSAGE(message_name) \
    /* create feature getters for the message */ \
    ::dynamix::feature& _dynamix_get_mixin_feature_safe(const I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)*) \
    { \
        return ::dynamix::internal::feature_instance<I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)>::the_feature_safe(); \
    } \
    const ::dynamix::feature& _dynamix_get_mixin_feature_fast(const I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)*) \
    { \
        return ::dynamix::internal::feature_instance<I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)>::the_feature_fast(); \
    } \
    /* create a feature registrator */ \
    void _dynamix_register_mixin_feature(const I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)*) \
    { \
        ::dynamix::internal::domain::safe_instance(). \
            register_feature(::dynamix::internal::feature_instance<I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)>::the_feature_safe()); \
    } \
    /* instantiate metafunction initializator in case no class registers the message */ \
    inline void _dynamix_register_message(I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)*) \
    { \
        ::dynamix::internal::message_registrator<I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name)>::registrator.unused = true; \
    } \
    /* provide a tag instance */ \
    I_DYNAMIX_MESSAGE_STRUCT_NAME(message_name) * I_DYNAMIX_MESSAGE_TAG(message_name)
