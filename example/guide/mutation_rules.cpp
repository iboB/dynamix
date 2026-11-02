// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/mutation-rules.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/mutation_rule_info.hpp>
#include <dynamix/type_mutation.hpp>

#include <iostream>

DYNAMIX_DECLARE_DOMAIN(game);

// [classes]
struct body {};
struct physics {
    float mass = 1;
    physics() = default;
    explicit physics(float m) : mass(m) {}
};
struct character {};
struct inventory {};
struct ghost {};
// [classes]

DYNAMIX_DECLARE_MIXIN(body);
DYNAMIX_DECLARE_MIXIN(physics);
DYNAMIX_DECLARE_MIXIN(character);
DYNAMIX_DECLARE_MIXIN(inventory);
DYNAMIX_DECLARE_MIXIN(ghost);

void print_type(const dynamix::object& obj) {
    std::cout << '{';
    const char* sep = "";
    for (auto m : obj.get_type().mixins) {
        std::cout << sep << m->name.to_std();
        sep = ", ";
    }
    std::cout << "}\n";
}

// [custom-rule]
// a custom rule: ghosts have no physics
dynamix::error_return_t no_physics_for_ghosts(dnmx_type_mutation_handle hmut, uintptr_t /*user_data*/) {
    auto& mut = *dynamix::type_mutation::from_c_handle(hmut);
    if (mut.has<ghost>()) {
        mut.remove<physics>();
    }
    return dynamix::result_success;
}

const dynamix::mutation_rule_info no_physics_for_ghosts_rule = {
    dnmx_make_sv_lit("no physics for ghosts"),
    no_physics_for_ghosts,
    0, // user data
    100 // order priority: apply after the default ones (0)
};
// [custom-rule]

int main() {
    using namespace dynamix;

    auto& dom = g::get_domain<game>();

    // [attaches-to]
    // physics attaches to body: it's added and removed together with it
    object obj(dom);
    mutate(obj, add<body>());
    print_type(obj); // {body, physics}

    mutate(obj, remove<body>());
    print_type(obj); // {}

    // physics can't be added without a body
    mutate(obj, add<physics>());
    print_type(obj); // {}
    // [attaches-to]

    // [dep-args]
    // but if it will be added by a rule, its constructor arguments can be provided
    mutate(obj, add<body>(), add<physics>(5.f));
    print_type(obj); // {body, physics}
    std::cout << "mass: " << obj.get<physics>()->mass << '\n';
    // [dep-args]

    // [also-adds]
    // character also adds inventory, but doesn't remove it
    mutate(obj, add<character>());
    print_type(obj); // {body, character, inventory, physics}
    // note that physics, being a dependency, is always re-added at the back

    mutate(obj, remove<character>());
    print_type(obj); // {body, inventory, physics}
    // [also-adds]

    // [add-rule]
    dom.add_mutation_rule(no_physics_for_ghosts_rule);

    mutate(obj, add<ghost>());
    print_type(obj); // {body, inventory, ghost}

    mutate(obj, remove<ghost>());
    print_type(obj); // {body, inventory, physics}

    dom.remove_mutation_rule(no_physics_for_ghosts_rule);
    // [add-rule]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/common_mutation_rules.hpp>

// [define]
DYNAMIX_DEFINE_MIXIN(game, body);
DYNAMIX_DEFINE_MIXIN(game, physics)
    .on_mutation(attaches_to<body>{})
;
DYNAMIX_DEFINE_MIXIN(game, character)
    .on_mutation(also_adds<inventory>{})
;
DYNAMIX_DEFINE_MIXIN(game, inventory);
DYNAMIX_DEFINE_MIXIN(game, ghost);
// [define]

DYNAMIX_DEFINE_DOMAIN(game, "game");
