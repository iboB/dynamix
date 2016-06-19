// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "executable_pch.hpp"
#include "object_factory.hpp"
#include "../common/messages.hpp"
#include "../dynlib/dynlib_messages.hpp"

using namespace std;

typedef void (*plugin_modify_object_proc)(dynamix::object*);

plugin_modify_object_proc plugin_modify_object;

#if defined (WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef HMODULE DynamicLib;

#define LoadDynamicLib(lib) LoadLibrary(lib ".dll")
#define CloseDynamicLib FreeLibrary
#define GetProc GetProcAddress

#else

#include <dlfcn.h>

typedef void* DynamicLib;
#define LoadDynamicLib(lib) dlopen("lib" lib ".so", RTLD_NOW)
#define CloseDynamicLib dlclose
#define GetProc dlsym

#endif

void load_plugin()
{
    DynamicLib lib = LoadLibrary("plugin");
    DYNAMIX_ASSERT(lib);

    auto fp = GetProc(lib, "modify_object");
    DYNAMIX_ASSERT(fp);

    plugin_modify_object = reinterpret_cast<plugin_modify_object_proc>(fp);
}

void call_messages(dynamix::object* o)
{
    cout << " ====== trace ====== " << endl;
    trace(o, cout);

    cout << endl << " ====== dl unicast messages ====== " << endl;
    dl_mixin_specific(o);
    dl_exported(o);
    cout << endl << " ====== dl unicast multicast ====== " << endl;
    dl_multicast(o);
}

int main()
{
    object_factory f;

    set_id(f.the_object, 42);

    call_messages(f.the_object);

    cout << endl << endl << endl << " ====== loading plugin ====== " << endl;

    load_plugin();
    plugin_modify_object(f.the_object);

    call_messages(f.the_object);

    return 0;
}