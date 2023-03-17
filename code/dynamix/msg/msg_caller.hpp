// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../common_feature_info.hpp"
#include "../feature_payload.hpp"
#include "../exception.hpp"
#include "../type.hpp"

namespace dynamix {
// caller for a specific signature
// also templated by object to preserve const-ness
template <typename Object, typename Ret, typename... Args>
struct msg_caller {
    static Ret try_default_payload(const common_feature_info& info, Object& obj, Args&&... args) {
        if (!info.default_payload) throw bad_feature_access("dynamix message");

        // we have a default payload
        // note that it has a different signature (obj first instead of mixin)
        auto func = reinterpret_cast<Ret(*)(Object&, Args...)>(info.default_payload);
        return func(obj, std::forward<Args>(args)...);
    }

    using func_t = Ret(*)(const void*, Args...);

    static Ret call(const type::ftable_payload& pl, Object& obj, Args&&... args) {
        // here we just use `const void*` while we must technically copy the const-ness of the object
        // it's easier and more readable as is
        // let's see if it will bite us
        auto func = reinterpret_cast<func_t>(pl.payload);

        // unchecked mixin at index: we trust the macro and type generation
        auto mixin_data = obj.unchecked_get_at(pl.mixin_index);

        return func(mixin_data, std::forward<Args>(args)...);
    }

    static Ret call_unicast(const common_feature_info& info, Object& obj, Args&&... args) {
        const type& t = obj.get_type();

        auto fe = t.ftable_at(info.id); // ftable entry
        if /*likely*/ (fe) {
            auto& top = *fe.begin; // top bid
            return call(top, obj, std::forward<Args>(args)...);
        }

        return try_default_payload(info, obj, std::forward<Args>(args)...);
    }

    static Ret call_multicast(const common_feature_info& info, Object& obj, Args&&... args) {
        const type& t = obj.get_type();

        auto fe = t.ftable_at(info.id); // ftable entry
        if /*likely*/ (fe) {
            // reverse order of execution:
            // this way the same-prio multicast execution order follows the mixins order
            // and prio messages will higher prio be executed first
            for (auto i = fe.top_bid_back; i != fe.begin; --i) {
                // not reusing call here, because we need to topy the arguments
                auto func = reinterpret_cast<func_t>(i->payload);
                auto mixin_data = obj.unchecked_get_at(i->mixin_index);
                func(mixin_data, args...); // copy and ignore return value
            }
            // return first (top) result
            return call(*fe.begin, obj, std::forward<Args>(args)...);
        }

        return try_default_payload(info, obj, std::forward<Args>(args)...);
    }
};
}