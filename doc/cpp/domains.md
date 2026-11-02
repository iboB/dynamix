# Domains

A domain is an instance of the library, of sorts. It's a registry of [mixins](mixins.md), [features](messages.md), [mutation rules](mutation-rules.md), and [type classes](type-classes.md), and it creates and owns the [types](mutations.md#types) composed of them. Every object belongs to a domain, which is set when the object is created and never changes.

Most projects need a single domain. Multiple domains are useful for independent subsystems which use DynaMix in different ways, like game objects and UI widgets, or for parts of a project which need different domain settings.

> [!TIP]
> The code on this page is from [example/guide/domains.cpp](../../example/guide/domains.cpp). It's a complete program which is built and run with the library's tests.

## Contents

* [Declaring and defining domains](#declaring-and-defining-domains)
* [Settings](#settings)
* [Domains are separate](#domains-are-separate)
* [The domain as a registry](#the-domain-as-a-registry)
* [Thread safety](#thread-safety)
* [Domains without macros](#domains-without-macros)

## Declaring and defining domains

```c++
#include <dynamix/declare_domain.hpp>

// two domains for different parts of the application
DYNAMIX_DECLARE_DOMAIN(game);
DYNAMIX_DECLARE_DOMAIN(ui);
```

`DYNAMIX_DECLARE_DOMAIN(tag)` declares a *domain tag*: an incomplete struct which identifies the domain in code. The tag is used to define mixins in the domain and to get the domain instance:

```c++
domain& gd = g::get_domain<game>();
domain& ud = g::get_domain<ui>();
```

The domain is defined once, in a single compilation unit:

```c++
#include <dynamix/define_domain.hpp>
#include <dynamix/domain_settings_builder.hpp>

DYNAMIX_DEFINE_DOMAIN(game, "game", dynamix::domain_settings_builder{}.canonicalize_types());
DYNAMIX_DEFINE_DOMAIN(ui, "ui");
```

The arguments after the tag are optional and are passed to the constructor of `dynamix::domain`:

| Argument | Description |
|---|---|
| `name` | A name for diagnostics. It appears in error messages and in debug dumps. Not used for anything else. |
| `settings` | The [domain settings](#settings). |
| `user_data` | A `uintptr_t` value which the library doesn't touch, accessible as `domain::user_data`. |
| `context` | A `void*` which the library doesn't touch, accessible as `domain::context`. |
| `alloc` | The allocator for the domain's internal data. See [Allocators](../advanced/allocators.md). |

The domain instance is a function-local static, created on the first call to `g::get_domain`. This is typically during the static initialization of the first mixin which is defined in it. Because of this, the order of static initialization across compilation units is not a problem. The domain is destroyed when the program exits, after the mixins which were registered in it.

When the domain is defined in a shared library and used from outside of it, use `DYNAMIX_DECLARE_EXPORTED_DOMAIN(export_macro, tag)`. See [Multiple Domains, Shared Libraries, and Plugins](../advanced/plugins.md).

## Settings

`dynamix::domain_settings` is a struct of flags. All are `false` by default. `domain_settings_builder` allows setting them with a chain of method calls:

```c++
dynamix::domain_settings_builder{}
    .canonicalize_types()
    .allow_duplicate_mixin_names()
```

### `canonicalize_types`

By default the order of mixins in a type is the order in which they were added. When a domain canonicalizes types, mixins are sorted in a *canonical order* instead, and the order in which they were added doesn't matter:

```c++
// the game domain canonicalizes types:
// the order of mixins doesn't depend on the order in which they're added
object a(gd), b(gd);
mutate(a, add<transform>(), add<health>(), add<ai>());
mutate(b, add<ai>(), add<health>(), add<transform>());
print_type(a.get_type()); // {transform, ai, health}
std::cout << "same type: " << (&a.get_type() == &b.get_type()) << '\n'; // true

// the ui domain doesn't
object c(ud), d(ud);
mutate(c, add<button>(), add<label>());
mutate(d, add<label>(), add<button>());
print_type(c.get_type()); // {button, label}
print_type(d.get_type()); // {label, button}
```

The canonical order is by the mixin's `order_priority` (lower first) and then by name:

```c++
// order_priority is used to sort mixins in domains which canonicalize types
// mixins with the same order_priority are sorted by name
DYNAMIX_DEFINE_MIXIN(game, transform).order_priority(-1);
DYNAMIX_DEFINE_MIXIN(game, health);
DYNAMIX_DEFINE_MIXIN(game, ai);
```

Canonical types mean fewer types: objects with the same set of mixins always have the same type. They also make the order of mixins predictable regardless of how objects were built, for example when they're loaded from files. On the other hand [overriding by order](messages.md#by-order) is no longer practical: features must be overridden with bids and priorities.

This was the only behavior in DynaMix v1.

### `allow_duplicate_mixin_names` and `allow_duplicate_feature_names`

By default, registering a mixin or a feature whose name is already registered in the domain is an error. These settings allow duplicate names. In such domains, operations by name are unreliable, since a name may refer to one of several mixins or features.

## Domains are separate

Objects, types, mixins, and features belong to a single domain. Using a mixin from one domain in an object of another throws `domain_error`:

```c++
// mixins can't be mixed between domains
try {
    mutate(c, add<health>());
}
catch (const domain_error& e) {
    std::cout << "error: " << e.what() << '\n';
}
```

```
error: ui: foreign mixin 'health' from 'game' while trying to create type {'button', 'label', 'health'}
```

A class can be defined as a mixin with `DYNAMIX_DEFINE_MIXIN` in only one domain. To use the same class as a mixin in multiple domains, you need a separate `mixin_info` for each domain. See [Mixins Without Macros](../advanced/no-macros.md).

Likewise, a feature, like a message, can only be used in one domain. The first domain in which a mixin which implements it is registered owns it. Registering a mixin which implements a feature that's owned by another domain throws `domain_error`, and nothing is registered.

Registration errors like this one, or like duplicate names, are usually programming errors. When they happen during static initialization (which is where `DYNAMIX_DEFINE_MIXIN` registers mixins), there's nothing to catch the exception and the program is terminated. Most debuggers and IDEs can break on uncaught exceptions, which shows where the error happened. When mixins are registered explicitly, for example by a plugin loader, the exception can be caught to reject the plugin without crashing the application.

## The domain as a registry

A domain knows all of its mixins and features, and they can be looked up by name or by id:

```c++
if (auto info = gd.get_mixin_info("health")) {
    std::cout << "found mixin: " << info->name.to_std() << '\n';
}
```

`get_mixin_info`, `get_feature_info`, and `get_type_class` return `nullptr` if nothing matches.

`dynamix::domain_traverse` from [domain_traverse.hpp](../../code/dynamix/domain_traverse.hpp) allows iterating over everything in a domain: mixins, features, types, mutation rules, and type classes:

```c++
domain_traverse(gd).traverse_mixins([](const mixin_info& info) {
    std::cout << "game mixin: " << info.name.to_std() << '\n';
});
```

This, together with the queries of types and objects, gives complete runtime reflection of everything in a domain. See [Reflection and Introspection](../advanced/reflection.md) and [Debugging](../working-with/debugging.md).

### Types

The domain creates types when they're first needed and caches them. Types which no longer have objects are not destroyed automatically, so that they can be reused. `garbage_collect_types()` destroys them:

```c++
// types without objects are kept, until they're garbage collected
{
    object tmp(gd);
    mutate(tmp, add<health>());
}
std::cout << "types: " << gd.num_types() << '\n'; // 2
gd.garbage_collect_types();
std::cout << "types after gc: " << gd.num_types() << '\n'; // 1
```

Every domain also has an *empty type*: the type of objects with no mixins, available through `get_empty_type()`. It implements the features which have [default implementations](messages.md#default-implementations).

### Registration

`DYNAMIX_DEFINE_MIXIN` registers mixins automatically, and mixins register the features which they implement. Mixins and features can also be registered and unregistered manually with `register_mixin`, `unregister_mixin`, `register_feature`, and `unregister_feature`. The domain doesn't copy the infos, so they must outlive their registration.

Unregistering a mixin destroys all types which contain it. Objects of such types must be destroyed or mutated before that. Using them afterwards (even destroying them) is undefined behavior. This is mostly relevant for [plugins](../advanced/plugins.md) which are unloaded.

Mutation rules and type classes are also registered in domains. See [Mutation Rules](mutation-rules.md) and [Type Classes](type-classes.md).

## Thread safety

A domain is internally synchronized. Registering and unregistering mixins and features, getting types, and lookups by name or id can all be done from multiple threads at the same time.

Objects are not synchronized. Different objects can be used and mutated from different threads, but an object which is being mutated can't be used from another thread at the same time. See [Thread Safety](../working-with/thread-safety.md).

## Domains without macros

A `dynamix::domain` is a regular class. It can be created without the macros, for example as a local variable or as a member of another class:

```c++
dynamix::domain dom("local", settings);
```

Such domains can't be used with `DYNAMIX_DEFINE_MIXIN`, which requires a domain tag. Mixins have to be registered in them manually. This is useful for tests, tools, and for domains whose mixins are created at run time, like ones defined by scripts or data. See [Mixins Without Macros](../advanced/no-macros.md).

A domain can't be copied or moved, and it must outlive all of its objects.
