// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "basic.hpp"
#include "gl_renderer.hpp"
#include "rendering_messages.hpp"
#include "transform_messages.hpp"
#include "system_messages.hpp"

using namespace std;

void gl_renderer::render() const
{
    render(0);
}

void gl_renderer::render(int render_target) const
{
    int transform = get_combined_transform(dm_this);

    // main_devince->SetTransform(glTS_WORLD, transform);

    cout << "OpenGL rendering object " << get_id(dm_this) << endl
         << "\ton target " << render_target << endl
         << "\twith transformation: " << transform << endl
         << "\t" << (_casts_shadows ? "" : "not ") << "casting shadows" << endl;
}

void gl_renderer::trace(std::ostream& o) const
{
    o << "\twith a OpenGL renderer" << endl;
}

DYNAMIX_DEFINE_MIXIN(gl_renderer, all_rendering_messages & trace_msg & opengl_specific_msg & opengl_specific_multicast_msg);
