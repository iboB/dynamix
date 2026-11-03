// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/cpp/domains.md
//
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>

#include <dynamix/domain.hpp>
#include <dynamix/domain_traverse.hpp>
#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/exception.hpp>

#include <iostream>

// [declare]
// two domains for different parts of the application
DYNAMIX_DECLARE_DOMAIN(game);
DYNAMIX_DECLARE_DOMAIN(ui);
// [declare]

// game mixins
DYNAMIX_DECLARE_MIXIN(struct transform);
DYNAMIX_DECLARE_MIXIN(struct health);
DYNAMIX_DECLARE_MIXIN(struct ai);

// ui mixins
DYNAMIX_DECLARE_MIXIN(struct button);
DYNAMIX_DECLARE_MIXIN(struct label);

void print_type(const dynamix::type& t) {
    std::cout << '{';
    const char* sep = "";
    for (auto m : t.mixins) {
        std::cout << sep << m->name.to_std();
        sep = ", ";
    }
    std::cout << "}\n";
}

int main() {
    using namespace dynamix;
    std::cout << std::boolalpha;

    // [get]
    domain& gd = g::get_domain<game>();
    domain& ud = g::get_domain<ui>();
    std::cout << gd.name() << " canonicalizes types: " << gd.settings().canonicalize_types << '\n';
    std::cout << ud.name() << " canonicalizes types: " << ud.settings().canonicalize_types << '\n';
    // [get]

    // [canonical]
    // the game domain canonicalizes types:
    // the order of mixins doesn't depend on the order in which they're added
    object a(gd), b(gd);
    mutate(a, add<transform>(), add<health>(), add<ai>());
    mutate(b, add<ai>(), add<health>(), add<transform>());
    print_type(a.get_type()); // {transform, ai, health}
    std::cout << "same type: " << (&a.get_type() == &b.get_type()) << '\n';

    // the ui domain doesn't
    object c(ud), d(ud);
    mutate(c, add<button>(), add<label>());
    mutate(d, add<label>(), add<button>());
    print_type(c.get_type()); // {button, label}
    print_type(d.get_type()); // {label, button}
    // [canonical]

    // [foreign]
    // mixins can't be mixed between domains
    try {
        mutate(c, add<health>());
    }
    catch (const domain_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    // [foreign]

    // [registry]
    // the domain is a registry of mixins, features, and types
    if (auto info = gd.get_mixin_info("health")) {
        std::cout << "found mixin: " << info->name.to_std() << '\n';
    }

    domain_traverse(gd).traverse_mixins([](const mixin_info& info) {
        std::cout << "game mixin: " << info.name.to_std() << '\n';
    });
    // [registry]

    // [gc]
    // types without objects are kept, until they're garbage collected
    {
        object tmp(gd);
        mutate(tmp, add<health>());
    }
    std::cout << "types: " << gd.num_types() << '\n';
    gd.garbage_collect_types();
    std::cout << "types after gc: " << gd.num_types() << '\n';
    // [gc]

    return 0;
}

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/domain_settings_builder.hpp>

struct transform {};
struct health {};
struct ai {};
struct button {};
struct label {};

// [define-mixins]
// order_priority is used to sort mixins in domains which canonicalize types
// mixins with the same order_priority are sorted by name
DYNAMIX_DEFINE_MIXIN(game, transform).order_priority(-1);
DYNAMIX_DEFINE_MIXIN(game, health);
DYNAMIX_DEFINE_MIXIN(game, ai);

DYNAMIX_DEFINE_MIXIN(ui, button);
DYNAMIX_DEFINE_MIXIN(ui, label);
// [define-mixins]

// [define]
DYNAMIX_DEFINE_DOMAIN(game, "game", dynamix::domain_settings_builder{}.canonicalize_types());
DYNAMIX_DEFINE_DOMAIN(ui, "ui");
// [define]
