// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "transform_mixin.hpp"

#include <iosfwd>

class has_transform
{
public:
    has_transform();

    int get_combined_transform() const { return _transform; }

    void translate(int to);
    void rotate(int with);

    void trace(std::ostream& o) const;

private:
    int _transform;
};
