// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/advanced/reflection.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <dynamix/domain.hpp>
#include <dynamix/domain_traverse.hpp>
#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/feature_info.hpp>
#include <dynamix/dbg_dmp.hpp>

#include <iostream>
#include <string>
#include <vector>

DYNAMIX_DECLARE_DOMAIN(game);

DYNAMIX_DECLARE_MSG(update_msg, update, void, (dynamix::object&, float));
DYNAMIX_DECLARE_MSG(render_msg, render, void, (const dynamix::object&));

int main() {
    using namespace dynamix;
    std::cout << std::boolalpha;
    auto& dom = g::get_domain<game>();

    // [traverse]
    // everything that's registered in the domain
    // (the traverse object locks the domain, so it's in a scope)
    {
        domain_traverse trav(dom);
        trav.traverse_mixins([](const mixin_info& m) {
            std::cout << "mixin '" << m.name.to_std() << "' implements:";
            for (auto& f : m.features_span()) {
                std::cout << ' ' << f.info->name.to_std();
            }
            std::cout << '\n';
        });
    }
    // [traverse]

    // [by-name]
    // compose an object from names, for example read from a file
    std::vector<std::string> composition = {"transform", "mesh", "ai"};
    object obj(dom);
    for (auto& name : composition) {
        mutate(obj, add(name));
    }
    // [by-name]

    // [inspect]
    // inspect an object
    const type& t = obj.get_type();
    for (mixin_index_t i = 0; i < t.num_mixins(); ++i) {
        std::cout << i << ": " << t.mixins[i]->name.to_std() << '\n';
    }
    std::cout << "implements render: " << t.implements_strong("render_msg") << '\n';
    std::cout << "implements update: " << t.implements_strong("update_msg") << '\n';

    // which mixin implements update?
    auto entry = t.ftable_at(update_msg::info.id);
    std::cout << "update is implemented by: " << t.mixins[entry.begin->mixin_index]->name.to_std() << '\n';
    // [inspect]

    // [dump]
    util::dbg_dmp(std::cout, obj);
    std::cout << '\n';
    // [dump]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

struct transform {};
struct mesh { void render() const {} };
struct ai { void update(float) {} };

DYNAMIX_MAKE_FUNC_TRAITS(update);
DYNAMIX_MAKE_FUNC_TRAITS(render);

DYNAMIX_DEFINE_MIXIN(game, transform);
DYNAMIX_DEFINE_MIXIN(game, mesh).implements<render_msg>();
DYNAMIX_DEFINE_MIXIN(game, ai).implements<update_msg>();

DYNAMIX_DEFINE_MSG(update_msg, multicast, update, void, (dynamix::object&, float));
DYNAMIX_DEFINE_MSG(render_msg, unicast, render, void, (const dynamix::object&));

DYNAMIX_DEFINE_DOMAIN(game, "game");
