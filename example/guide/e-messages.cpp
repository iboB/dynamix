// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/messages.md
//
// A character in a game is a dynamix::object. Mixins add traits, forms, and
// status effects to it, and messages let the rest of the code use the
// character without knowing what it's composed of.
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
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// declarations: this is what users of the character need to see

DYNAMIX_DECLARE_DOMAIN(game);

// [declare-msgs]
// unicast: one implementation is executed per call
DYNAMIX_DECLARE_MSG(get_name_msg, get_name, std::string, (const dynamix::object&));
DYNAMIX_DECLARE_MSG(speak_msg, speak, std::string, (const dynamix::object&));

// multicast: all (top-bid) implementations are executed per call
DYNAMIX_DECLARE_MSG(describe_msg, describe, void, (const dynamix::object&, std::vector<std::string>&));

// a non-const message: implementations can modify the mixins
DYNAMIX_DECLARE_MSG(rename_msg, rename, void, (dynamix::object&, std::string));
// [declare-msgs]

DYNAMIX_DECLARE_MIXIN(struct person);
DYNAMIX_DECLARE_MIXIN(struct human_voice);
DYNAMIX_DECLARE_MIXIN(struct dragon_form);
DYNAMIX_DECLARE_MIXIN(struct silenced);
DYNAMIX_DECLARE_MIXIN(struct shouting);
DYNAMIX_DECLARE_MIXIN(struct impostor);

void print_description(const dynamix::object& obj) {
    std::vector<std::string> parts;
    describe(obj, parts);
    std::cout << get_name(obj) << " is:";
    for (auto& p : parts) std::cout << ' ' << p << ';';
    std::cout << '\n';
}

int main() {
    using namespace dynamix;

    object hero(g::get_domain<game>());

    // [default-impl]
    // the object is empty, but get_name has a default implementation
    std::cout << get_name(hero) << '\n';

    // speak has none, so calling it throws
    try {
        speak(hero);
    }
    catch (const feature_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    // [default-impl]

    // [unicast]
    mutate(hero, add<person>("Alice"), add<human_voice>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';
    // [unicast]

    // [non-const]
    rename(hero, "Alicia");
    std::cout << "renamed to " << get_name(hero) << '\n';
    // [non-const]

    // [override-by-order]
    // dragon_form also implements speak_msg with the same bid
    // the last mixin in the type wins
    mutate(hero, add<dragon_form>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';
    // [override-by-order]

    // [override-by-bid]
    // silenced implements speak_msg with a higher bid
    // it overrides speak regardless of the mixin order
    mutate(hero, add<silenced>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';

    // removing the status effect restores the previous behavior
    mutate(hero, remove<silenced>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';
    // [override-by-bid]

    // [next-impl]
    // shouting has the highest bid and calls the next implementer
    mutate(hero, add<shouting>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';
    mutate(hero, remove<dragon_form>());
    std::cout << get_name(hero) << " says: " << speak(hero) << '\n';
    // [next-impl]

    // [multicast]
    mutate(hero, add<dragon_form>(), add<silenced>());
    print_description(hero);
    // [multicast]

    // [multicast-priority]
    // person is added last here, but its describe has a higher priority
    // so it is still executed first
    object villain(g::get_domain<game>());
    mutate(villain, add<dragon_form>(), add<silenced>(), add<person>("Smaug"));
    print_description(villain);
    // [multicast-priority]

    // [clash]
    // get_name_msg does not allow clashes:
    // impostor implements it with the same bid and priority as person
    try {
        mutate(hero, add<impostor>());
    }
    catch (const type_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    // [clash]

    // [query]
    // `implements` is true for features with a default implementation, too
    // `implements_strong` is only true if a mixin implements it
    object nobody(g::get_domain<game>());
    const type& t = nobody.get_type();
    std::cout << std::boolalpha
        << "nobody implements get_name: " << t.implements<get_name_msg>()
        << ", strong: " << t.implements_strong<get_name_msg>() << '\n'
        << "hero implements speak: " << hero.get_type().implements<speak_msg>() << '\n';
    // [query]

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// definitions: they don't need to be visible to users of the character
// they could just as well be in other compilation units, or in plugins

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/msg/next_impl.hpp>

#include <cctype>

// [mixins]
struct person {
    std::string name;
    person(std::string n) : name(std::move(n)) {}

    std::string get_name() const { return name; }
    void rename(std::string n) { name = std::move(n); }
    void describe(std::vector<std::string>& out) const {
        out.push_back("a person");
    }
};

struct human_voice {
    std::string speak() const { return "Hello"; }
};

struct dragon_form {
    std::string speak() const { return "Roar"; }
    void describe(std::vector<std::string>& out) const {
        out.push_back("in the form of a dragon");
    }
};

struct silenced {
    void describe(std::vector<std::string>& out) const {
        out.push_back("silenced");
    }
};

struct shouting {
    std::string speak() const {
        // call the implementation of speak_msg which is next in line after this mixin
        auto str = DYNAMIX_CALL_NEXT_IMPL_MSG(speak_msg);
        for (auto& c : str) c = char(std::toupper(c));
        return str + "!";
    }
};

struct impostor {};
// [mixins]

// [define-mixins]
DYNAMIX_DEFINE_MIXIN(game, person)
    .implements<get_name_msg>()
    .implements<rename_msg>()
    .implements<describe_msg>(1_prio) // higher priority: described first
;

DYNAMIX_DEFINE_MIXIN(game, human_voice)
    .implements<speak_msg>()
;

DYNAMIX_DEFINE_MIXIN(game, dragon_form)
    .implements<speak_msg>()
    .implements<describe_msg>()
;

DYNAMIX_DEFINE_MIXIN(game, silenced)
    // implement a message with a function or lambda instead of a method
    // the first argument is a pointer to the mixin
    .implements_by<speak_msg>([](const silenced*) -> std::string { return "..."; }, 1_bid)
    .implements<describe_msg>()
;

DYNAMIX_DEFINE_MIXIN(game, shouting)
    .implements<speak_msg>(2_bid)
;

DYNAMIX_DEFINE_MIXIN(game, impostor)
    .implements_by<get_name_msg>([](const impostor*) -> std::string { return "Bob"; })
;
// [define-mixins]

// [define-msgs]
DYNAMIX_MAKE_FUNC_TRAITS(get_name);
DYNAMIX_MAKE_FUNC_TRAITS(speak);
DYNAMIX_MAKE_FUNC_TRAITS(describe);
DYNAMIX_MAKE_FUNC_TRAITS(rename);

// default implementation of get_name
// it is called with the object as the first argument (not a mixin)
std::string default_name(const dynamix::object&) { return "Someone"; }

// unicast, which does not allow clashes, and has a default implementation
DYNAMIX_DEFINE_MSG_EX(get_name_msg, unicast, false, default_name,
    get_name, std::string, (const dynamix::object&));

DYNAMIX_DEFINE_MSG(speak_msg, unicast, speak, std::string, (const dynamix::object&));
DYNAMIX_DEFINE_MSG(describe_msg, multicast, describe, void, (const dynamix::object&, std::vector<std::string>&));
DYNAMIX_DEFINE_MSG(rename_msg, unicast, rename, void, (dynamix::object&, std::string));
// [define-msgs]

DYNAMIX_DEFINE_DOMAIN(game, "game");
