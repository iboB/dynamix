// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "has_id_mixin.hpp"

class has_id
{
public:
    int get_id() const { return _id; }
    void set_id(int id) { _id = id; }

    void trace(std::ostream& o) const;

private:
    int _id;
};
