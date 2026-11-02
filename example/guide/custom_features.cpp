// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/advanced/custom-features.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/any.hpp>
#include <dynamix/common_feature_info.hpp>

#include <iostream>

DYNAMIX_DECLARE_DOMAIN(game);

// [declare]
// a custom feature: a value associated with a mixin
// types which have this feature have a max speed
struct max_speed_traits;
struct max_speed {
    using traits = max_speed_traits;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};
// [declare]

// [traits]
// feature traits: they produce the payloads of features for mixins
struct max_speed_traits {
    // used by implements_with<max_speed>(value)
    // the payload is a copy of the value, owned by the mixin's definition
    static dynamix::any make_payload_with(const dynamix::allocator& alloc, void* /*mixin*/, float value) {
        return dynamix::make_any(alloc, value);
    }
};
// [traits]

// [access]
// accessing the feature: no object needed, it works on types
float get_max_speed(const dynamix::type& t) {
    auto entry = t.ftable_at(max_speed::info.id);
    if (entry) {
        // the top implementation
        return *static_cast<const float*>(entry.begin->payload);
    }
    // the default payload
    return *static_cast<const float*>(max_speed::info.default_payload);
}

float get_max_speed(const dynamix::object& obj) {
    return get_max_speed(obj.get_type());
}
// [access]

DYNAMIX_DECLARE_MIXIN(struct walker);
DYNAMIX_DECLARE_MIXIN(struct car);
DYNAMIX_DECLARE_MIXIN(struct turbo);

int main() {
    using namespace dynamix;
    auto& dom = g::get_domain<game>();

    // [use]
    object obj(dom);
    std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 0: default

    mutate(obj, add<walker>());
    std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 5

    mutate(obj, remove<walker>(), add<car>());
    std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 50

    mutate(obj, add<turbo>());
    std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 80
    // [use]

    // [type]
    // types can be queried without objects
    const mixin_info* infos[] = {&g::get_mixin_info<walker>(), &g::get_mixin_info<turbo>()};
    auto& t = dom.get_type(infos);
    std::cout << "walker with turbo: " << get_max_speed(t) << '\n'; // 80
    // [type]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>

// [define]
const dynamix::common_feature_info& max_speed::get_info_safe() {
    static float default_max_speed = 0;
    static dynamix::common_feature_info the_info(
        dnmx_make_sv_lit("max_speed"), // name
        false, // don't allow clashes
        &default_max_speed // default payload
    );
    return the_info;
}
const dynamix::feature_info& max_speed::info = max_speed::get_info_safe();
// [define]

// [mixins]
struct walker {};
struct car {};
struct turbo {};

DYNAMIX_DEFINE_MIXIN(game, walker).implements_with<max_speed>(5.f);
DYNAMIX_DEFINE_MIXIN(game, car).implements_with<max_speed>(50.f);
DYNAMIX_DEFINE_MIXIN(game, turbo).implements_with<max_speed>(80.f, 1_bid);
// [mixins]

DYNAMIX_DEFINE_DOMAIN(game, "game");
