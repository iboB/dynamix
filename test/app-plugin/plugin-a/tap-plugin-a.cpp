// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <tap-object.hpp>
#include <tap-domain.hpp>
#include <tap-messages.hpp>
#include <tap-messages-traits.hpp>

#include <dynamix/mutate.hpp>
#include <dynamix/define_mixin.hpp>

#include <splat/symbol_export.h>

struct plugin_a_mixin {
    std::string work(char) const {
        return "hardly working";
    }
    void dump(std::vector<std::string>& vec) const {
        vec.push_back("plugin-a");
    }
};

DYNAMIX_DEFINE_MIXIN(tap::dynamix_domain, plugin_a_mixin)
    .implements<tap::work_msg>(-1_bid)
    .implements<tap::dump_msg>();

extern "C" {
SYMBOL_EXPORT void modify_object(tap::object& obj) {
    mutate(obj, dynamix::add<plugin_a_mixin>());
}
SYMBOL_EXPORT void release_object(tap::object& obj) {
    mutate(obj, dynamix::remove<plugin_a_mixin>());
}
}