// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/objects.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/exception.hpp>

#include <iostream>
#include <string>

DYNAMIX_DECLARE_DOMAIN(game);

// [entity]
// a class derived from dynamix::object
// it provides a domain-specific constructor, copy semantics, and stronger typing for messages
class entity : public dynamix::object {
public:
    entity() : dynamix::object(dynamix::g::get_domain<game>()) {}

    // copying
    entity(const entity& other) : dynamix::object(other.copy()) {}
    entity& operator=(const entity& other) {
        copy_from(other);
        return *this;
    }

    // the copy operations above hide the defaulted move operations, so we bring them back
    entity(entity&&) noexcept = default;
    entity& operator=(entity&&) = default;
};

// messages can take the derived class instead of dynamix::object
DYNAMIX_DECLARE_MSG(describe_msg, describe, std::string, (const entity&));
// [entity]

// [classes]
struct transform {
    float x = 0, y = 0;
    std::string describe() const {
        return "at " + std::to_string(int(x)) + "," + std::to_string(int(y));
    }
};
struct health {
    int hp = 100;
};
// [classes]

DYNAMIX_DECLARE_MIXIN(transform);
DYNAMIX_DECLARE_MIXIN(health);

int main() {
    using namespace dynamix;
    std::cout << std::boolalpha;

    // [create]
    entity hero;
    std::cout << "empty: " << hero.empty() << '\n'; // true
    mutate(hero, add<transform>(), add<health>());
    // [create]

    // [query]
    // by type: returns a typed pointer or null
    if (auto* t = hero.get<transform>()) t->x = 5;

    // by name or by mixin info: returns void* or null
    std::cout << "has health: " << hero.has("health") << '\n';
    auto* h = static_cast<health*>(hero.get("health"));
    h->hp = 80;

    // by index in the type
    std::cout << "mixins: " << hero.num_mixins() << '\n';
    std::cout << "first: " << hero.get_type().mixins[0]->name.to_std() << '\n';
    // [query]

    // [msg]
    std::cout << describe(hero) << '\n';
    // [msg]

    // [copy]
    entity clone = hero; // copy constructor: clone has the same type and copies of the mixins
    std::cout << "clone hp: " << clone.get<health>()->hp << '\n';

    entity other;
    mutate(other, add<health>());
    other = hero; // copy_from: other changes its type to the one of hero
    std::cout << "other has transform: " << other.has<transform>() << '\n';
    // [copy]

    // [matching]
    // copy only the mixins which both objects have
    entity patient;
    mutate(patient, add<health>());
    patient.copy_matching_from(hero);
    std::cout << "patient hp: " << patient.get<health>()->hp << ", has transform: " << patient.has<transform>() << '\n';
    // [matching]

    // [move]
    // moving an object doesn't move its mixins
    auto* ht = hero.get<transform>();
    entity moved = std::move(hero);
    std::cout << "same transform: " << (moved.get<transform>() == ht) << '\n';
    std::cout << "moved-from is empty: " << hero.empty() << '\n';
    // [move]

    // [clear]
    moved.clear();
    std::cout << "cleared: " << moved.empty() << '\n';
    // [clear]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

DYNAMIX_MAKE_FUNC_TRAITS(describe);

DYNAMIX_DEFINE_MIXIN(game, transform).implements<describe_msg>();
DYNAMIX_DEFINE_MIXIN(game, health);

DYNAMIX_DEFINE_MSG(describe_msg, unicast, describe, std::string, (const entity&));

DYNAMIX_DEFINE_DOMAIN(game, "game");
