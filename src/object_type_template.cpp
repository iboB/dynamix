// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/object_type_template.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/object.hpp>

using namespace std;

namespace dynamix
{

using namespace internal;

object_type_template::object_type_template()
    : object_mutator(object_type_info::null().as_mixin_collection())
{
}

void object_type_template::apply_to(object& o) const
{
    o.clear();
    object_mutator::apply_to(o);
}

} // namespace dynamix
