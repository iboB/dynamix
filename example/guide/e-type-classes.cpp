// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/type-classes.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/declare_type_class.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>

#include <iostream>
#include <vector>

DYNAMIX_DECLARE_DOMAIN(game);

DYNAMIX_DECLARE_MIXIN(struct transform);
DYNAMIX_DECLARE_MIXIN(struct mesh);
DYNAMIX_DECLARE_MIXIN(struct sprite);
DYNAMIX_DECLARE_MIXIN(struct body);

DYNAMIX_DECLARE_MSG(render_msg, render, void, (const dynamix::object&));

// [declare]
DYNAMIX_DECLARE_TYPE_CLASS(renderable);
DYNAMIX_DECLARE_TYPE_CLASS(physical);
// [declare]

int main() {
    using namespace dynamix;
    std::cout << std::boolalpha;

    auto& dom = g::get_domain<game>();

    std::vector<object> objects;
    objects.emplace_back(dom);
    mutate(objects.back(), add<transform>(), add<mesh>(), add<body>());
    objects.emplace_back(dom);
    mutate(objects.back(), add<transform>(), add<sprite>());
    objects.emplace_back(dom);
    mutate(objects.back(), add<transform>(), add<body>());

    // [use]
    for (auto& obj : objects) {
        if (obj.is_of<renderable>()) render(obj);
    }

    // type classes work on types, too
    for (auto& obj : objects) {
        auto& t = obj.get_type();
        std::cout << "physical: " << t.is_of<physical>() << '\n';
    }

    // and they can be referred to by name
    std::cout << "renderable: " << objects[2].is_of("renderable") << '\n';
    // [use]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/define_type_class.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

struct transform {};
struct mesh {
    void render() const { std::cout << "rendering a mesh\n"; }
};
struct sprite {
    void render() const { std::cout << "rendering a sprite\n"; }
};
struct body {};

DYNAMIX_MAKE_FUNC_TRAITS(render);

DYNAMIX_DEFINE_MIXIN(game, transform);
DYNAMIX_DEFINE_MIXIN(game, mesh).implements<render_msg>();
DYNAMIX_DEFINE_MIXIN(game, sprite).implements<render_msg>();
DYNAMIX_DEFINE_MIXIN(game, body);

DYNAMIX_DEFINE_MSG(render_msg, unicast, render, void, (const dynamix::object&));

// [define]
// a type class is defined by a function which checks a type
bool is_renderable(const dynamix::type& t) {
    return t.implements_strong<render_msg>();
}
DYNAMIX_DEFINE_TYPE_CLASS_WITH(game, renderable, is_renderable);

// ... or by a captureless lambda
DYNAMIX_DEFINE_TYPE_CLASS_WITH(game, physical, [](const dynamix::type& t) {
    return t.has<transform>() && t.has<body>();
});
// [define]

DYNAMIX_DEFINE_DOMAIN(game, "game");