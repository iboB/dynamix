// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "mutation_rule.hpp"
#include "common_mutation_rules.hpp"
#include "domain.hpp"

#include <dynamix/domain.hpp>
#include <dynamix/mixin_info.hpp>
#include <dynamix/throw_exception.hpp>

#include <itlib/data_mutex.hpp>
#include <itlib/qalgorithm.hpp>

#include <mutex>

namespace dynamix::v1compat {

namespace {
error_return_t mutation_rule_apply_to(dnmx_type_mutation_handle mutation, uintptr_t user_data) {
    auto mut = type_mutation::from_c_handle(mutation);
    auto rule = reinterpret_cast<mutation_rule*>(user_data);
    rule->apply_to(*mut);
    return result_success;
}
struct registry {
    std::vector<std::shared_ptr<mutation_rule>> rules;
};
itlib::data_mutex<registry, std::mutex> the_registry;
}

mutation_rule::mutation_rule() noexcept {
    m_info.name = {};
    m_info.apply = mutation_rule_apply_to;
    m_info.order_priority = 0;
    m_info.user_data = reinterpret_cast<uintptr_t>(this);
}

mutation_rule::~mutation_rule() = default;

mutation_rule_id add_mutation_rule(std::shared_ptr<mutation_rule> rule) {
    auto l = the_registry.unique_lock();
    auto& rules = l->rules;
    auto r = itlib::pfind(rules, nullptr);
    if (!r) {
        r = &rules.emplace_back();
    }

    auto& dom = domain::instance();
    dom.add_mutation_rule(rule->info());
    *r = std::move(rule);
    return mutation_rule_id(r - rules.data());
}

mutation_rule_id add_mutation_rule(mutation_rule* rule) {
    return add_mutation_rule(std::shared_ptr<mutation_rule>(rule));
}

std::shared_ptr<mutation_rule> remove_mutation_rule(mutation_rule_id id) {
    auto l = the_registry.unique_lock();
    auto& rules = l->rules;
    if (id >= rules.size()) return {};
    auto r = std::move(rules[id]);
    if (!r) return {};
    auto& dom = domain::instance();
    dom.remove_mutation_rule(r->info());
    return r;
}

void dependent_mixins_oneshot::apply_to(type_mutation& mut) {
    if (!mut.has(*master)) return;
    for (auto i : infos) {
        mut.add_if_lacking(*i);
    }
}

void dependent_mixins_dep::apply_to(type_mutation& mut) {
    if (!mut.has(*master)) return;
    for (auto i : infos) {
        if (!i->dependency) dynamix::throw_exception::type_mut_error(mut, "rule with no dep", *i);
        mut.add_if_lacking(*i);
    }
}

void mutually_exclusive_mixins::apply_to(type_mutation& mut) {
    // we want to keep the last
    // so reverse, keep the first, then reverse again
    std::reverse(mut.mixins.begin(), mut.mixins.end());
    bool found = false;
    itlib::erase_all_if(mut.mixins, [&](const mixin_info* i) {
        auto f = itlib::pfind(infos, i);
        if (!f) return false; // don't care about this one
        if (found) return true; // erase if another mutually exclusive was found
        found = true;
        return false; // keep the first
    });
    std::reverse(mut.mixins.begin(), mut.mixins.end());
}

}
