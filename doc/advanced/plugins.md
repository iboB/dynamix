# Shared Libraries and Plugins

DynaMix is designed to work across shared libraries. A project can have its core in one library, and other libraries, including plugins which are loaded at run time, can add mixins which implement the core's features. Objects created by the core can be composed with mixins from plugins, and code in the core which calls messages for these objects will execute implementations from the plugins, without knowing that they exist.

> [!TIP]
> [test/app-plugin](../../test/app-plugin/) is a complete example: a core library, an executable, and plugins which are loaded and unloaded at run time. It's built and run with the library's tests.

## Contents

* [Exporting](#exporting)
* [A typical structure](#a-typical-structure)
* [Plugins](#plugins)
* [Hotswap](#hotswap)
* [Multiple domains](#multiple-domains)

## Exporting

Everything which is declared in one shared library and used from another must be exported. All declaration macros have `EXPORTED` variants which take an export macro (typically one which expands to `__declspec(dllexport)`/`__declspec(dllimport)` on Windows and to a visibility attribute elsewhere) as their first argument:

| Macro | Exported variant |
|---|---|
| `DYNAMIX_DECLARE_DOMAIN(tag)` | `DYNAMIX_DECLARE_EXPORTED_DOMAIN(export, tag)` |
| `DYNAMIX_DECLARE_MIXIN(mixin)` | `DYNAMIX_DECLARE_EXPORTED_MIXIN(export, mixin)` |
| `DYNAMIX_DECLARE_MSG(...)` | `DYNAMIX_DECLARE_EXPORTED_MSG(export, ...)` |
| `DYNAMIX_DECLARE_SIMPLE_MSG(...)` | `DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(export, ...)` |
| `DYNAMIX_DECLARE_TYPE_CLASS(tc)` | `DYNAMIX_DECLARE_EXPORTED_TYPE_CLASS(export, tc)` |

The definition macros are the same for exported and non-exported declarations.

Only what's used across libraries needs to be exported. Mixins which are only referred to within their library (or only by name) don't need to be exported.

## A typical structure

The core library has:

* The domain: `DYNAMIX_DECLARE_EXPORTED_DOMAIN` in a header, `DYNAMIX_DEFINE_DOMAIN` in a source file.
* The messages (and other features) which make up the interface of objects: `DYNAMIX_DECLARE_EXPORTED_MSG` in a header, the function traits (`DYNAMIX_MAKE_FUNC_TRAITS`) in another header, and the definitions in a source file. See [Organizing code](../cpp/messages.md#organizing-code).
* Optionally a class derived from `dynamix::object`.
* Core mixins. Those which are used directly by other libraries are declared with `DYNAMIX_DECLARE_EXPORTED_MIXIN`.

Other libraries link with the core library, include its headers, and define mixins in its domain which implement its messages:

```c++
#include <tap-object.hpp>
#include <tap-domain.hpp>
#include <tap-messages.hpp>
#include <tap-messages-traits.hpp>

#include <dynamix/mutate.hpp>
#include <dynamix/define_mixin.hpp>

struct plugin_a_mixin {
    std::string work(char) const {
        return "hardly working";
    }
    void dump(std::vector<std::string>& vec) const {
        vec.push_back("plugin-a");
    }
};

DYNAMIX_DEFINE_MIXIN(tap::dynamix_domain, plugin_a_mixin)
    .implements<tap::work_msg>(-1_bid)
    .implements<tap::dump_msg>();
```

## Plugins

A plugin is a shared library which is loaded at run time (with `dlopen` or `LoadLibrary`). The mixins defined in it with `DYNAMIX_DEFINE_MIXIN` are registered in their domain when the plugin is loaded, and unregistered when it's unloaded.

A plugin typically exports some functions with C linkage, which the application calls after loading it. In the app-plugin test, a plugin exports functions which add its mixins to objects and remove them:

```c++
extern "C" {
SYMBOL_EXPORT void modify_object(tap::object& obj) {
    mutate(obj, dynamix::add<plugin_a_mixin>());
}
SYMBOL_EXPORT void release_object(tap::object& obj) {
    mutate(obj, dynamix::remove<plugin_a_mixin>());
}
}
```

Another approach is to not have such functions at all, and have the application add mixins from plugins by name: `mutate(obj, add("plugin_a_mixin"))`. This allows the composition of objects to be driven by data, like configuration files or saved games, which may refer to mixins from any plugin.

There are some rules for plugins:

* **Remove a plugin's mixins from all objects before unloading it.** Unloading the plugin unregisters its mixins, and unregistering a mixin destroys all types which contain it. Objects of these types become invalid and using them, even destroying them, is undefined behavior. Which objects to release is up to the application: it can keep track of them, or iterate its objects and check their types.
* **Plugins should not define new features (including messages).** They should implement the existing features of the application. When a mixin is registered, the domain also registers the features it implements, but unregistering the mixin doesn't unregister them. If a feature is defined in a plugin, unloading the plugin leaves the domain with a dangling pointer to its info, which leads to crashes when features are registered or looked up by name later. If a plugin must define a feature, it should unregister it explicitly with `domain::unregister_feature`, after the mixins which implement it are unregistered.
* **Registration errors in a plugin can't be caught**, since the registration with `DYNAMIX_DEFINE_MIXIN` happens during the plugin's static initialization, inside `dlopen`/`LoadLibrary`. If you need to recover from bad plugins (say, with duplicate mixin names), have the plugins register their mixins explicitly from a function which the application calls in a `try` block. See [Mixins Without Macros](no-macros.md).

## Hotswap

Since mixins can be removed from objects and unregistered, and new ones can be registered and added at any time, a plugin can be unloaded and replaced with a new version while the application is running. This allows changing the behavior of objects without restarting the application. It's especially useful during development: modify the code of a plugin, rebuild it, and reload it.

```c++
// "hotswap"
pa.release_object(obj);
close_plugin(pa);
pa = open_plugin("plugin-a-mod");
pa.modify_object(obj);
```

`plugin-a-mod` is a modified version of `plugin-a`. Its mixin has the same name, but a different implementation. After the swap, the object behaves differently, and code which works with it through messages doesn't need to change.

Removing a mixin from an object destroys it. To preserve state across a hotswap, save it before releasing the objects and restore it after the new plugin adds its mixins, for example with a serialization message, or by adding the new mixins with [constructor arguments or functions](../cpp/mutations.md#constructing-mixins).

## Multiple domains

A project can have multiple domains, each in a different library. They're completely independent: a mixin, a feature, or an object belongs to a single domain. See [Domains](../cpp/domains.md).

Multiple domains can be useful for separating subsystems which use DynaMix independently, or which need different domain settings. It's not necessary for plugins: plugins can (and typically should) use the application's domain.