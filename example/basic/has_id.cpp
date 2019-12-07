// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "has_id.hpp"
#include "system_messages.hpp"

#include <dynamix/define_mixin.hpp>
#include <iostream>

using namespace dynamix;

void has_id::trace(std::ostream& o) const
{
    o << "object with id: " << _id << std::endl;
}

//                                                    set the priority of trace to a big number so it would be called first
DYNAMIX_DEFINE_MIXIN(has_id, get_id_msg & set_id_msg & priority(10, trace_msg));
