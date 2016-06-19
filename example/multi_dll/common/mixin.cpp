// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "common_pch.hpp"
#include "messages.hpp"
#include "mixin.hpp"

using namespace dynamix;
using namespace std;

common_mixin::common_mixin()
    : _id(0)
{
}

void common_mixin::set_id(int id)
{
    _id = id;
}

int common_mixin::get_id() const
{
    return _id;
}

void common_mixin::trace(std::ostream& o) const
{
    o << "object with id " << _id << endl;
}

DYNAMIX_DEFINE_MIXIN(common_mixin, get_id_msg & set_id_msg & priority(1000, trace_msg));
