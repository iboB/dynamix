// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "basic.hpp"
#include "transform.hpp"
#include "system_messages.hpp"
#include "transform_messages.hpp"

using namespace dynamix;
using namespace std;

has_transform::has_transform()
    : _transform(0)
{
}

void has_transform::translate(int to)
{
    _transform += to;
}

void has_transform::rotate(int with)
{
    _transform *= with;
}

void has_transform::trace(std::ostream& o) const
{
    o << "\twith transform: " << _transform << endl;
}

DYNAMIX_DEFINE_MIXIN(has_transform, translate_msg & rotate_msg & trace_msg & get_combined_transform_msg);
