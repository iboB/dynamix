// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <tap-object.hpp>
#include <tap-core-mixins.hpp>
#include <tap-messages.hpp>

#include <dynamix/mutate.hpp>

#include <doctest/doctest.h>

using strvec = std::vector<std::string>;

TEST_CASE("pre plugins") {
    using namespace tap;
    object obj;
    mutate(obj, mixin_person("alice", 32), dynamix::add<teacher>());

    {
        strvec d;
        dump(obj, d);
        CHECK(d == strvec{"alice 32", "teacher"});
    }

    CHECK_THROWS_WITH_AS(work(obj, 'a'), "no target", std::runtime_error);
    set_target(obj, "bob");

    CHECK(work(obj, 'a') == "teaching bob a");
}

#if defined (_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef HMODULE hplugin;

inline hplugin load_plugin(std::string_view path, std::string_view lib) {
    std::string l{path};
    l += '/';
#if defined(__GNUC__)
    l += "lib";
#endif
    l += lib;
    l += ".dll";
    return LoadLibraryA(l.c_str());
}

#define unload_plugin FreeLibrary
#define get_proc GetProcAddress

#else

#include <dlfcn.h>

typedef void* hplugin;

inline hplugin load_plugin(std::string_view path, std::string_view lib) {
    std::string l{path};
    l += "/lib";
    l += lib;
#if defined(__APPLE__)
    l += ".dylib";
#else
    l += ".so";
#endif
    return dlopen(l.c_str(), RTLD_NOW | RTLD_LOCAL);
}
#define unload_plugin dlclose
#define get_proc dlsym

#endif

struct plugin {
    hplugin handle = nullptr;
    using proc = void (*)(tap::object& obj);
    proc modify_object = nullptr;
    proc release_object = nullptr;
};

plugin open_plugin(std::string name) {
    name = "dynamix-test-plugin-" + name;
    plugin ret;
    ret.handle = load_plugin(PLUGIN_DIR, name);
    REQUIRE(ret.handle);
    ret.modify_object = reinterpret_cast<plugin::proc>(get_proc(ret.handle, "modify_object"));
    REQUIRE(!!ret.modify_object);
    ret.release_object = reinterpret_cast<plugin::proc>(get_proc(ret.handle, "release_object"));
    REQUIRE(!!ret.release_object);
    return ret;
}

void close_plugin(plugin& plugin) {
    unload_plugin(plugin.handle);
    plugin = {};
}

TEST_CASE("load plugins") {
    tap::object obj;
    mutate(obj, tap::mixin_person("bob", 40));
    CHECK(work(obj, 'z') == "working with z");

    auto pa = open_plugin("plugin-a");
    pa.modify_object(obj);

    CHECK(work(obj, '?') == "hardly working");

    {
        strvec d;
        dump(obj, d);
        CHECK(d == strvec{"bob 40", "plugin-a"});
    }

    auto pb = open_plugin("plugin-b");
    pb.modify_object(obj);

    {
        strvec d;
        dump(obj, d);
        CHECK(d == strvec{"bob 40", "plugin-a", "plugin-b"});
    }

    // "hotswap"
    pa.release_object(obj);
    close_plugin(pa);
    pa = open_plugin("plugin-a-mod");
    pa.modify_object(obj);

    {
        strvec d;
        dump(obj, d);
        CHECK(d == strvec{"bob 40", "plugin-b", "plugin-a-mod"});
    }

    pb.release_object(obj);
    pa.release_object(obj);
    close_plugin(pa);
    close_plugin(pb);
}
