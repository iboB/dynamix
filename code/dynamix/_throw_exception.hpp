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
namespace priv {
[[noreturn]] void throw_id_registered(const domain& dom, const feature_info& info);
[[noreturn]] void throw_id_registered(const domain& dom, const mixin_info& info);
[[noreturn]] void throw_empty_name(const domain& dom, const feature_info& info);
[[noreturn]] void throw_empty_name(const domain& dom, const mixin_info& info);
[[noreturn]] void throw_empty_name(const domain& dom, const type_class& tc);
[[noreturn]] void throw_duplicate_name(const domain& dom, const feature_info& info);
[[noreturn]] void throw_duplicate_name(const domain& dom, const mixin_info& info);
[[noreturn]] void throw_duplicate_name(const domain& dom, const type_class& tc);
[[noreturn]] void throw_info_has_domain(const domain& dom, const mixin_info& info);
[[noreturn]] void throw_unreg_foreign(const domain& dom, const feature_info& info);
[[noreturn]] void throw_unreg_foreign(const domain& dom, const mixin_info& info);
[[noreturn]] void throw_no_func(const domain& dom, const mutation_rule_info& info);
[[noreturn]] void throw_no_func(const domain& dom, const type_class& tc);
[[noreturn]] void throw_mutation_rule_user_error(const type_mutation& mut, const mutation_rule_info& info, error_return_t error);
[[noreturn]] void throw_foreign_domain(const domain& dom, const type_mutation& mut);
[[noreturn]] void throw_cyclic_rule_deps(const type_mutation& mut);
[[noreturn]] void throw_mut_unreg_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void throw_mut_foreign_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void throw_mut_dup_mixin(const type_mutation& mut, const mixin_info& m);
[[noreturn]] void throw_feature_clash(const type_mutation& mut, const dnmx_ftable_payload& a, const dnmx_ftable_payload& b);
}
}
