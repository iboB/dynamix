// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <dynamix/dynamix.hpp>
#include "../dynlib/dynlib_messages.hpp"

using namespace std;
using namespace dynamix;

DYNAMIX_DECLARE_MIXIN(plugin_mixin);

extern "C" DYNAMIX_SYMBOL_EXPORT void modify_object(object* o)
{
    mutate(o).add<plugin_mixin>();
}

extern "C" DYNAMIX_SYMBOL_EXPORT void release_object(object* o)
{
    mutate(o).remove<plugin_mixin>();
}

class plugin_mixin
{
public:
    void trace(ostream& o) const
    {
        cout << "\twith a plugin mixin" << endl;
    }

    void dl_multicast()
    {
        cout << "dynlib multicast in plugin mixin" << endl;
    }

    void dl_mixin_specific()
    {
        cout << "the plugin has overriden the dynlib specific message" << endl;
    }
};

DYNAMIX_DEFINE_MIXIN(plugin_mixin, trace_msg & priority(10, dl_mixin_specific_msg) & dl_multicast_msg);
