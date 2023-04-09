// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_info_fwd.hpp"
#include "mixin_info_fwd.hpp"
#include "mutation_rule_info.hpp"
#include "type_class.hpp"

struct dnmx_ftable_payload;

namespace dynamix {
class domain;
class type_mutation;
namespace throw_exception {
[[noreturn]] void id_registered(const domain& dom, const feature_info& info);
[[noreturn]] void id_registered(const domain& dom, const mixin_info& info);
[[noreturn]] void empty_name(const domain& dom, const feature_info& info);
[[noreturn]] void empty_name(const domain& dom, const mixin_info& info);
[[noreturn]] void empty_name(const domain& dom, const type_class& tc);
[[noreturn]] void duplicate_name(const domain& dom, const feature_info& info);
[[noreturn]] void duplicate_name(const domain& dom, const mixin_info& info);
[[noreturn]] void duplicate_name(const domain& dom, const type_class& tc);
[[noreturn]] void info_has_domain(const domain& dom, const mixin_info& info);
[[noreturn]] void unreg_foreign(const domain& dom, const feature_info& info);
[[noreturn]] void unreg_foreign(const domain& dom, const mixin_info& info);
[[noreturn]] void no_func(const domain& dom, const mutation_rule_info& info);
[[noreturn]] void no_func(const domain& dom, const type_class& tc);
[[noreturn]] void mutation_rule_user_error(const type_mutation& mut, const mutation_rule_info& info, error_return_t error);
[[noreturn]] void foreign_domain(const domain& dom, const type_mutation& mut);
[[noreturn]] void cyclic_rule_deps(const type_mutation& mut);
[[noreturn]] void mut_unreg_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void mut_foreign_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void mut_dup_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void feature_clash(const type_mutation& mut, const dnmx_ftable_payload& a, const dnmx_ftable_payload& b);
}
}
