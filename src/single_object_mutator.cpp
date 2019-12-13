// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/single_object_mutator.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/mixin_type_info.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/object.hpp>

using namespace std;

namespace dynamix
{

using namespace internal;

single_object_mutator::single_object_mutator(object* o)
    : _object(*o)
    , _is_manually_applied(false)
{
    I_DYNAMIX_ASSERT(o);
}

single_object_mutator::single_object_mutator(object& o)
    : _object(o)
    , _is_manually_applied(false)
{
}

single_object_mutator::~single_object_mutator()
{
    if(!_is_manually_applied)
    {
        // allow single line mutations
        apply();
    }
}

void single_object_mutator::apply()
{
    _source_mixins = _object._type_info->as_mixin_collection();
    create();
    apply_to(_object);
    cancel(); // to go back to empty state
    _source_mixins = nullptr; // really empty
    _is_manually_applied = true;
}

} // namespace dynamix
