// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Functions and classes related to mutation rules.
 */

#include "config.hpp"
#include "mixin_collection.hpp"
#include "mutation_rule_id.hpp"

#include <memory>

namespace dynamix
{

class object_type_mutation;

/// Base class for all mutation rules
class DYNAMIX_API mutation_rule
{
public:
    virtual ~mutation_rule() {}

    /// Called when applying the mutation rule.
    virtual void apply_to(object_type_mutation& mutation) = 0;
};

/// DEPRECATED:
/// Adds a mutation rule to the domain.
/// Takes ownership of the pointer and assumes it's created with `new`.
/// Returns the mutation rule id by whitch it can be removed.
///
/// Does *not* perform a topological sort of the rules.
/// It is the user's responsibility to add the mutation rules in the appropriate order.
mutation_rule_id DYNAMIX_API add_new_mutation_rule(mutation_rule* rule);

/// Adds a mutation rule to the domain.
/// Takes ownership of the pointer and assumes it's created with `new`.
/// Returns the mutation rule id by whitch it can be removed.
///
/// Does *not* perform a topological sort of the rules.
/// It is the user's responsibility to add the mutation rules in the appropriate order.
mutation_rule_id DYNAMIX_API add_mutation_rule(mutation_rule* rule);

/// Adds a mutation rule to the domain via a shared pointer.
/// Returns the mutation rule id by whitch it can be removed.
///
/// Does *not* perform a topological sort of the rules.
/// It is the user's responsibility to add the mutation rules in the appropriate order.
mutation_rule_id DYNAMIX_API add_mutation_rule(std::shared_ptr<mutation_rule> rule);

/// Removes a mutation rule from the domain by id.
/// Returns a shared pointer to the rule which the user might use to persist it, or null
/// if no such mutation rule exists
std::shared_ptr<mutation_rule> DYNAMIX_API remove_mutation_rule(mutation_rule_id id);

} // namespace dynamix
