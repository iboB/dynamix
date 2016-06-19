// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "dynlib_mixin_fwd.hpp"

class dynlib_mixin
{
public:
    void trace(std::ostream& o) const;

    void dl_multicast();
    void dl_mixin_specific();
};