// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include <dynamix/mutation_rule_info.hpp>
#include <memory>

namespace dynamix {
class type_mutation;
namespace v1compat {
class DYNAMIX_V1COMPAT_API mutation_rule {
    mutation_rule_info m_info;
public:
    mutation_rule() noexcept;

    mutation_rule(const mutation_rule&) = delete;
    mutation_rule& operator=(const mutation_rule&) = delete;

    virtual ~mutation_rule();
    virtual void apply_to(type_mutation& mutation) = 0;

    const mutation_rule_info& info() const noexcept { return m_info; };
};

using mutation_rule_id = uint32_t;
DYNAMIX_V1COMPAT_API mutation_rule_id add_mutation_rule(mutation_rule* rule);
DYNAMIX_V1COMPAT_API mutation_rule_id add_mutation_rule(std::shared_ptr<mutation_rule> rule);
DYNAMIX_V1COMPAT_API std::shared_ptr<mutation_rule> remove_mutation_rule(mutation_rule_id id);
}
}
