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

/// The same type object mutator mutates objects of the same type.
/// Namely, it adds and removes mixins from objects that initially have the same mixins.
///
/// If you want to mutate many objects of the same type, using this class
/// will make it much faster than using the `single_object_mutator` multiple times.
///
///
class DYNAMIX_API same_type_mutator : public internal::object_mutator
{
public:
    same_type_mutator();
    same_type_mutator(const internal::object_type_info* info);

    // not using but hiding parent's function
    void apply_to(object& o);

private:
    // create must be called by apply_to
    using object_mutator::create;
};

} // namespace dynamix
