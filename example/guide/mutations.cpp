// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/mutations.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/type_mutation.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/exception.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

DYNAMIX_DECLARE_DOMAIN(game);

// [classes]
struct transform {
    float x = 0, y = 0;
};

struct health {
    int hp;
    explicit health(int max) : hp(max) {}
};

struct ai {
    std::string script;
    ai() = default;
    explicit ai(std::string s) : script(std::move(s)) {}
};

struct cursed {
    explicit cursed(bool fail) {
        if (fail) throw std::runtime_error("the curse failed");
    }
};
// [classes]

DYNAMIX_DECLARE_MIXIN(transform);
DYNAMIX_DECLARE_MIXIN(health);
DYNAMIX_DECLARE_MIXIN(ai);
DYNAMIX_DECLARE_MIXIN(cursed);

void print_type(const dynamix::object& obj) {
    std::cout << '{';
    const char* sep = "";
    for (auto m : obj.get_type().mixins) {
        std::cout << sep << m->name.to_std();
        sep = ", ";
    }
    std::cout << "}\n";
}

int main() {
    using namespace dynamix;

    auto& dom = g::get_domain<game>();
    std::cout << std::boolalpha;

    // [basic]
    object obj(dom);
    mutate(obj, add<transform>(), add<health>(100));
    print_type(obj); // {transform, health}

    mutate(obj, remove<transform>(), add<ai>("patrol.lua"));
    print_type(obj); // {health, ai}
    // [basic]

    // [by-info-name]
    // mixins can be referred to by mixin_info or by name, too
    // (note that remove by name must be qualified: see doc/cpp/mutations.md)
    mutate(obj, dynamix::remove("ai"), add(g::get_mixin_info<transform>()));
    print_type(obj); // {health, transform}
    // [by-info-name]

    // [func]
    // the construction of a mixin can be customized by a function
    mutate(obj, add(g::get_mixin_info<ai>(), [](init_new_args args) {
        new (args.mixin_buf) ai("custom.lua");
    }));
    std::cout << obj.get<ai>()->script << '\n';
    // [func]

    // [to-back]
    mutate(obj, to_back<health>());
    print_type(obj); // {transform, ai, health}
    // [to-back]

    // [errors]
    // adding a mixin which is already in the object is an error
    try {
        mutate(obj, add<ai>());
    }
    catch (const type_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }

    // so is referring to a mixin by a name which doesn't exist
    try {
        mutate(obj, add("flying"));
    }
    catch (const type_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }

    // removing a mixin which is not in the object is not an error
    mutate(obj, remove<cursed>());
    // [errors]

    // [add-if-lacking]
    std::string script = "guard.lua";
    mutate(obj, add_if_lacking<ai>(std::move(script)));
    // obj already has ai, so script was not moved from
    std::cout << "script: " << script << '\n';
    // [add-if-lacking]

    // [exception-safety]
    // if a mixin constructor throws, the object is not changed
    auto ai_ptr = obj.get<ai>();
    try {
        mutate(obj, remove<ai>(), add<cursed>(true));
    }
    catch (const std::runtime_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    print_type(obj); // {transform, ai, health}
    std::cout << "same ai: " << (obj.get<ai>() == ai_ptr) << '\n';
    // [exception-safety]

    // [types]
    // objects with the same mixins share the same type
    object other(dom);
    mutate(other, add<transform>(), add<ai>(), add<health>(5));
    std::cout << "same type: " << (&obj.get_type() == &other.get_type()) << '\n';

    // types can be created without objects
    type_mutation tmut(dom);
    tmut.add<transform>();
    tmut.add<ai>();
    const type& t = dom.get_type(std::move(tmut));

    // and objects can be created from types (with default construction)
    object npc(t);
    print_type(npc); // {transform, ai}

    // or mutated to a type, optionally constructing some of the mixins with arguments
    mutate_to(npc, other.get_type(), construct<health>(30));
    print_type(npc); // {transform, ai, health}
    std::cout << "hp: " << npc.get<health>()->hp << '\n';
    // [types]

    // [gradual]
    // a simpler mutation for mixins which don't need constructor arguments
    // it's applied when the returned value is destroyed (at the semicolon)
    mutate(npc)
        .remove<ai>()
        .add_if_lacking<transform>()
        .add<ai>();
    print_type(npc); // {transform, health, ai}
    // [gradual]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>

DYNAMIX_DEFINE_MIXIN(game, transform);
DYNAMIX_DEFINE_MIXIN(game, health);
DYNAMIX_DEFINE_MIXIN(game, ai);
DYNAMIX_DEFINE_MIXIN(game, cursed);

DYNAMIX_DEFINE_DOMAIN(game, "game");
