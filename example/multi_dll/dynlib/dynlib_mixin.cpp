// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "dynlib_pch.hpp"
#include "dynlib_mixin.hpp"
#include "dynlib_messages.hpp"

using namespace std;

void dynlib_mixin::trace(std::ostream& o) const
{
    o << "\twith dynlib_mixin" << std::endl;
}

void dynlib_mixin::dl_mixin_specific()
{
    cout << "dynlib mixin reporting in" << endl;
}

void dynlib_mixin::dl_multicast()
{
    cout << "dynlib multicast in dynlib mixin" << endl;
}

DYNAMIX_DEFINE_MIXIN(dynlib_mixin, trace_msg & dl_mixin_specific_msg & dl_multicast_msg);
