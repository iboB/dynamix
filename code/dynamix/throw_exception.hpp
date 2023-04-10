// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "feature_info_fwd.hpp"
#include "mixin_info_fwd.hpp"
#include "mutation_rule_info.hpp"
#include "type_class.hpp"

#include <string_view>

struct dnmx_ftable_payload;

namespace dynamix {
class domain;
class type_mutation;
class type;
namespace throw_exception {
// domain_error
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
[[noreturn]] void foreign_mutation(const domain& dom, const type_mutation& mut);
[[noreturn]] void foreign_mixin(const type_mutation& mut, const mixin_info& m);

// type_error
[[noreturn]] void mutation_rule_user_error(const type_mutation& mut, const mutation_rule_info& info, error_return_t error);
[[noreturn]] void cyclic_rule_deps(const type_mutation& mut);
[[noreturn]] void type_mut_error(const type_mutation& mut, std::string_view err, const mixin_info& m);
[[noreturn]] void type_mut_error(const type_mutation& mut, std::string_view op, std::string_view mixin_name);
[[noreturn]] void feature_clash(const type_mutation& mut, const dnmx_ftable_payload& a, const dnmx_ftable_payload& b);
[[noreturn]] void unknown_type_class(const type& t, const std::string_view name);

// object_error
[[noreturn]] void obj_mut_error(const type& t, std::string_view op, std::string_view err, const mixin_info& m);
[[noreturn]] void obj_mut_user_error(const type& t, std::string_view op, std::string_view ovr, const mixin_info& m, error_return_t error);
[[noreturn]] void obj_mut_sealed_object(const type& t, std::string_view op);
[[noreturn]] void obj_error(const type& t, std::string_view op);
}
}
