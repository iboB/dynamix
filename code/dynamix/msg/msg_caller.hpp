// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../feature_info.hpp"
#include "../feature_payload.hpp"
#include "../exception.hpp"
#include "../type.hpp"
#include "../../dnmx/bits/no_sanitize.h"

namespace dynamix {
// caller for a specific signature
// also templated by object to preserve const-ness
template <typename Object, typename Ret, typename... Args>
struct msg_caller {
    static Ret try_default_payload(const feature_info& info, Object& obj, Args&&... args) {
        if (!info.default_payload) throw bad_feature_access("dynamix message");

        // we have a default payload
        // note that it has a different signature (obj first instead of mixin)
        auto func = reinterpret_cast<Ret(*)(Object&, Args...)>(info.default_payload);
        return func(obj, std::forward<Args>(args)...);
    }

    using func_t = Ret(*)(const void*, Args...);

	template <typename... CallArgs> // different arguments to allow casts when calling an individual payload
    I_DYNAMIX_NO_SANITIZE("undefined")
    // so, we don't sanitize the following function for ub
    // we do do two instances of ub here, but it's safe and no compiler can make use of it and
    // ruin our day:
    // * the major one is that we allow a function of type `ret(Mixin* mixin_ptr, args...)` to be
    //   assigned as a message payload and we cast it to `ret(const void* mixin_ptr, args...)` here
    //   this is safe: there is no difference between Mixin* and const void* on assembly level
    //   moreover there is not way for a compiler to know what we do with the function pointer when it's
    //   added to a feature_for_mixin instance (and thus it cannot elliminate the call)
    // * the minor one is that we cast to const void* indiscriminately while we technically must copy
    //   the const-ness of the object
    //   this, unlike the major one, is fixable but since we're doing this ub anyway, why bother?
    //   moreover this disregard of constness, allows us to assign a non-const func to a const message
    static Ret call(const type::ftable_payload& pl, Object& obj, CallArgs&&... args) {
        auto func = reinterpret_cast<func_t>(pl.payload);

        // unchecked mixin at index: we trust the macro and type generation
        auto mixin_data = obj.unchecked_get_at(pl.mixin_index);

        return func(mixin_data, std::forward<CallArgs>(args)...);
    }

    static Ret call_unicast(const feature_info& info, Object& obj, Args&&... args) {
        const type& t = obj.get_type();

        auto fe = t.ftable_at(info.id); // ftable entry
        if /*likely*/ (fe) {
            auto& top = *fe.begin; // top bid
            return call(top, obj, std::forward<Args>(args)...);
        }

        return try_default_payload(info, obj, std::forward<Args>(args)...);
    }

    static Ret call_multicast(const feature_info& info, Object& obj, Args&&... args) {
        const type& t = obj.get_type();

        auto fe = t.ftable_at(info.id); // ftable entry
        if /*likely*/ (fe) {
            // reverse order of execution:
            // this way the same-prio multicast execution order follows the mixins order
            // and prio messages will higher prio be executed first
            for (auto i = fe.top_bid_back; i != fe.begin; --i) {
                call(*i, obj, args...); // args are copied! return value is ignored
            }
            // return first (top) result
            return call(*fe.begin, obj, std::forward<Args>(args)...);
        }

        return try_default_payload(info, obj, std::forward<Args>(args)...);
    }
};
}
