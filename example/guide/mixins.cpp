// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/mixins.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/mixin_info.hpp>
#include <dynamix/exception.hpp>

#include <iostream>

// [declare]
namespace game {
DYNAMIX_DECLARE_DOMAIN(world);

DYNAMIX_DECLARE_MIXIN(struct transform);
DYNAMIX_DECLARE_MIXIN(class health);
DYNAMIX_DECLARE_MIXIN(struct inventory);
DYNAMIX_DECLARE_MIXIN(struct script_state);
}
// [declare]

// [classes]
// the classes are only needed by code which accesses the mixins' members
// in a real project they would typically be in their own headers
#include <mutex>
#include <string>
#include <vector>

namespace game {

// a trivial mixin
struct transform {
    float x = 0, y = 0;
    bool operator==(const transform& other) const noexcept {
        return x == other.x && y == other.y;
    }
};

// a mixin with no default constructor
class health {
public:
    explicit health(int max) : m_hp(max) {}
    int hp() const { return m_hp; }
private:
    int m_hp;
};

// a regular mixin with members which are not trivial
struct inventory {
    std::vector<std::string> items;
};

// a mixin which can't be moved or copied (because of the mutex)
struct script_state {
    std::mutex mutex;
    std::string source;
};
}
// [classes]

void print_info(const dynamix::mixin_info& info) {
    std::cout << "'" << info.name.to_std() << "'"
        << " size: " << info.size
        << ", default constructible: " << !!info.init
        << ", copyable: " << (info.copy_init && info.copy_asgn)
        << ", external: " << info.external()
        << '\n';
}

int main() {
    using namespace dynamix;
    using namespace game;

    std::cout << std::boolalpha;

    // [info]
    print_info(g::get_mixin_info<transform>());
    print_info(g::get_mixin_info<health>());
    print_info(g::get_mixin_info<inventory>());
    print_info(g::get_mixin_info<script_state>());
    // [info]

    object hero(g::get_domain<world>());

    // [add]
    // health has no default constructor, so it must be added with arguments
    try {
        mutate(hero, add<health>());
    }
    catch (const object_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }

    mutate(hero, add<transform>(), add<health>(100));
    // [add]

    // [by-name]
    // mixins can be added by name, too (with default construction)
    mutate(hero, add("inventory"));
    std::cout << "hero has inventory: " << hero.has("inventory") << '\n';
    // [by-name]

    // [get]
    hero.get<transform>()->x = 5;
    hero.get<inventory>()->items.push_back("sword");
    std::cout << "hp: " << hero.get<health>()->hp() << '\n';
    // [get]

    // [equal]
    // transform uses operator== for equality checks
    object a(g::get_domain<world>()), b(g::get_domain<world>());
    mutate(a, add<transform>());
    mutate(b, add<transform>());
    std::cout << "equal: " << a.equals(b) << '\n';
    b.get<transform>()->y = 1;
    std::cout << "equal: " << a.equals(b) << '\n';
    // [equal]

    // [copy]
    // script_state is not copyable, so objects with it can't be copied
    mutate(hero, add<script_state>());
    try {
        auto copy = hero.copy();
    }
    catch (const object_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    // [copy]

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// definitions

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>

// [define]
namespace game {
// DYNAMIX_DEFINE_MIXIN must be in the namespace of the mixin
DYNAMIX_DEFINE_MIXIN(world, transform)
    .uses_equal() // use transform::operator==
;
DYNAMIX_DEFINE_MIXIN(world, health);
DYNAMIX_DEFINE_MIXIN(world, inventory);
DYNAMIX_DEFINE_MIXIN(world, script_state)
    .name("script")
;

DYNAMIX_DEFINE_DOMAIN(world, "world");
}
// [define]
