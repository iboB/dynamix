// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"
#include "object_mutator.hpp"

namespace dynamix
{

/// The single object mutator mutates objects.
/// Namely it adds and removes mixins from an object
/// While synchronizing the object's
/// type information
///
/// The usage is to call add and remove for the desired components and then
/// call `single_object_mutator::apply()` or simply have it leave the scope,
/// or use the typedef mutate to do the mutation on a single line
class DYNAMIX_API single_object_mutator : public internal::object_mutator
{
public:
    single_object_mutator(object* o);
    single_object_mutator(object& o);
    ~single_object_mutator();

    /// Applies the mutation to the object.
    void apply();

private:
    object& _object;
    bool _is_manually_applied;

    // create must be called by apply
    using object_mutator::create;
};

typedef single_object_mutator mutate;

} // namespace dynamix
