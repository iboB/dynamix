// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "object_manager.hpp"
#include "system_messages.hpp"
#include "transform_messages.hpp"
#include "rendering_messages.hpp"

#include <iostream>

// here we have only messages
// no mixin info whatsoever

using namespace dynamix;
using namespace std;

int main()
{
    object_manager m;

    m.create_objects();

    object& o = *m.objects()[rand()%m.objects().size()];

    trace(o, cout);

    rotate(o, 5);
    render(o, 7); // overload with target

    cout << "object implements opengl_specific_msg: " << boolalpha << o.implements(opengl_specific_msg) << endl;

    cout << endl << "======== changing rendering system ========" << endl << endl;
    m.change_rendering_sytem();

    rotate(o, 0);
    set_casts_shadows(o, false);
    trace(o, cout);
    rotate(o, 3);
    render(o); // overload with no specified target

    cout << "object implements opengl_specific_msg: " << boolalpha << o.implements(opengl_specific_msg) << endl;

    return 0;
}
