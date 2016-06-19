// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
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

#include "global.hpp"
#include "mixin_collection.hpp"

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

/// Adds a mutation rule to the domain.
///
/// Does *not* perform a topological sort of the rules.
/// It is the user's responsibility to add the mutation rules in the appropriate order.
void DYNAMIX_API add_new_mutation_rule(mutation_rule* rule);

} // namespace dynamix
