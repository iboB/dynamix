// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/same_type_mutator.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/mixin_type_info.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/object.hpp>

using namespace std;

namespace dynamix
{

same_type_mutator::same_type_mutator()
{
}

same_type_mutator::same_type_mutator(const object_type_info* info)
    : object_mutator(info->as_mixin_collection())
{
}


void same_type_mutator::apply_to(object& o)
{
    if(!_is_created)
    {
        _source_mixins = o._type_info->as_mixin_collection();
        create();
    }

    internal::object_mutator::apply_to(o);
}

} // namespace dynamix
