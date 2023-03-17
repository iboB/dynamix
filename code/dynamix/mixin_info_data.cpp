// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "mixin_info_data.hpp"
#include "domain.hpp"

namespace dynamix::util {

void mixin_info_data::register_in(domain& dom) {
    for (auto* r : mutation_rule_infos) {
        dom.add_mutation_rule(*r);
    }
    dom.register_mixin(info);
}

void mixin_info_data::unregister_from(domain& dom) noexcept {
    dom.unregister_mixin(info);
    for (auto* r : mutation_rule_infos) {
        dom.remove_mutation_rule(*r);
    }
}

}
