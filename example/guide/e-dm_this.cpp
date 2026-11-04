// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/dm-this.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

#include <iostream>
#include <string>

DYNAMIX_DECLARE_DOMAIN(game);

DYNAMIX_DECLARE_MIXIN(struct health);
DYNAMIX_DECLARE_MIXIN(struct name);
DYNAMIX_DECLARE_MIXIN(struct loot);

DYNAMIX_DECLARE_MSG(take_damage_msg, take_damage, void, (dynamix::object&, int));
DYNAMIX_DECLARE_MSG(on_death_msg, on_death, void, (dynamix::object&));

int main() {
    using namespace dynamix;

    object goblin(g::get_domain<game>());
    mutate(goblin, add<name>("Goblin"), add<health>(10), add<loot>());

    take_damage(goblin, 4);
    take_damage(goblin, 8);

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/dm_this.hpp>

// [mixins]
struct name {
    std::string value;
    explicit name(std::string v) : value(std::move(v)) {}
};

struct health {
    int hp;
    explicit health(int h) : hp(h) {}

    void take_damage(int dmg) {
        hp -= dmg;

        // access a sibling mixin through the object
        auto n = dm_this->get<name>();
        std::cout << (n ? n->value : "Something") << " has " << hp << " hp\n";

        if (hp <= 0) {
            // call a message for the object
            on_death(*dm_this);
        }
    }
};

struct loot {
    void on_death() {
        std::cout << "dropping loot\n";
    }
};
// [mixins]

DYNAMIX_MAKE_FUNC_TRAITS(take_damage);
DYNAMIX_MAKE_FUNC_TRAITS(on_death);

DYNAMIX_DEFINE_MIXIN(game, name);
DYNAMIX_DEFINE_MIXIN(game, health).implements<take_damage_msg>();
DYNAMIX_DEFINE_MIXIN(game, loot).implements<on_death_msg>();

DYNAMIX_DEFINE_MSG(take_damage_msg, unicast, take_damage, void, (dynamix::object&, int));
DYNAMIX_DEFINE_MSG(on_death_msg, multicast, on_death, void, (dynamix::object&));

DYNAMIX_DEFINE_DOMAIN(game, "game");
