// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "exe_mixin_fwd.hpp"

class exe_mixin
{
public:
    void trace(std::ostream& o) const;

    void dl_exported();
    void dl_multicast();
};