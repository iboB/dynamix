// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "gl_renderer_mixin.hpp"

#include <iosfwd>

class gl_renderer
{
public:
    gl_renderer() : _casts_shadows(true) {}

    void render() const;
    void render(int render_target) const;

    void set_casts_shadows(bool set) { _casts_shadows = set; }

    void trace(std::ostream& o) const;

    void opengl_specific() {}
    void opengl_specific_multicast() {}

private:
    bool _casts_shadows;
};
