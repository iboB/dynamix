// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "mixin_fwd.hpp"

class common_mixin
{
public:
    common_mixin();

    void trace(std::ostream& o) const;

    void set_id(int id);
    int get_id() const;

private:
    int _id;
};
