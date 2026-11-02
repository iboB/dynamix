# Defining Mixins

Mixins are the building blocks of DynaMix objects. In terms of multiple inheritance, they are the parents of a class. Unlike parents, though, they are composed at run time, and they don't need anything special in order to be composed.

A mixin is an ordinary C++ class. It doesn't need a common base class, virtual functions, or any DynaMix-specific code. Existing classes, including ones from third-party libraries, can be mixins. What makes a class a mixin is its *definition* with DynaMix: a registration in a [domain](domains.md), which describes the mixin to the library and lists the [features](messages.md) it implements.

> [!TIP]
> The code on this page is from [example/guide/mixins.cpp](../../example/guide/mixins.cpp). It's a complete program which is built and run with the library's tests.

## Contents

* [Declaring mixins](#declaring-mixins)
* [Defining mixins](#defining-mixins)
* [What DynaMix infers from the class](#what-dynamix-infers-from-the-class)
* [Names](#names)
* [Internal and external mixins](#internal-and-external-mixins)
* [Equality and comparison](#equality-and-comparison)
* [Builder reference](#builder-reference)
* [Mixins in shared libraries](#mixins-in-shared-libraries)

## Declaring mixins

```c++
#include <dynamix/declare_mixin.hpp>

namespace game {
DYNAMIX_DECLARE_DOMAIN(world);

DYNAMIX_DECLARE_MIXIN(struct transform);
DYNAMIX_DECLARE_MIXIN(class health);
DYNAMIX_DECLARE_MIXIN(struct inventory);
DYNAMIX_DECLARE_MIXIN(struct script_state);
}
```

`DYNAMIX_DECLARE_MIXIN` declares that a class is a mixin. It also works as a forward declaration of the class (thus the `struct` or `class` keyword). The class doesn't need to be defined at this point.

The declaration is all that's needed to add, remove, and query mixins:

```c++
mutate(obj, add<transform>(), remove<inventory>());
if (obj.has<health>()) { /*...*/ }
```

Code which accesses the members of a mixin, like `obj.get<health>()->hp()`, naturally needs the class definition, too. Code which only composes objects and calls messages doesn't. This means that in a project which mostly works through messages, most code doesn't include the definitions of mixins and doesn't need to be recompiled when they change.

## Defining mixins

```c++
#include <dynamix/define_mixin.hpp>

namespace game {
// DYNAMIX_DEFINE_MIXIN must be in the namespace of the mixin
DYNAMIX_DEFINE_MIXIN(world, transform)
    .uses_equal() // use transform::operator==
;
DYNAMIX_DEFINE_MIXIN(world, health);
DYNAMIX_DEFINE_MIXIN(world, inventory);
DYNAMIX_DEFINE_MIXIN(world, script_state)
    .name("script")
;
}
```

`DYNAMIX_DEFINE_MIXIN(domain_tag, mixin)` defines a mixin in a domain. The class must be complete at this point. The macro must appear exactly once in the program, in a single compilation unit, and in the namespace of the mixin class.

It evaluates to a *builder* on which you can call methods to describe the mixin further: list the features it implements, set its name, and so on. The available methods are listed in the [builder reference](#builder-reference). The definition ends with a semicolon, and a mixin with no features or customizations is just `DYNAMIX_DEFINE_MIXIN(world, health);`.

The definition registers the mixin in its domain during static initialization, before `main` is called, or when the shared library which contains it is loaded. When the program exits, or the shared library is unloaded, the mixin is unregistered. Unregistering a mixin also destroys all types which contain it. It's the responsibility of the user to make sure that no objects with such mixins exist at this point.

## What DynaMix infers from the class

The definition inspects the class and fills in how it's created, copied, moved, and destroyed. The capabilities of the class determine what can be done with objects which contain it:

| If the class... | then... |
|---|---|
| is default constructible | it can be added to objects without arguments: `add<transform>()` |
| is copy constructible and copy assignable | objects with it can be copied |
| is *nothrow* move constructible | it can be [internal](#internal-and-external-mixins) |
| is *nothrow* move assignable | objects with it can be move-assigned to with `object::move_matching_from` |
| has a non-trivial destructor | it's called when the mixin is removed or the object is destroyed |

```c++
print_info(g::get_mixin_info<transform>());
print_info(g::get_mixin_info<health>());
print_info(g::get_mixin_info<inventory>());
print_info(g::get_mixin_info<script_state>());
```

```
'transform' size: 8, default constructible: true, copyable: true, external: false
'health' size: 8, default constructible: false, copyable: true, external: false
'inventory' size: 24, default constructible: true, copyable: true, external: false
'script' size: 72, default constructible: true, copyable: false, external: true
```

(`g::get_mixin_info<Mixin>()` returns the [`mixin_info`](../../code/dnmx/mixin_info.h) of a mixin. It's the runtime description of the mixin which the library works with.)

None of these capabilities are required. A mixin which lacks one simply makes the corresponding operation impossible. Trying to do it anyway throws `dynamix::object_error`:

```c++
// health has no default constructor, so it must be added with arguments
try {
    mutate(hero, add<health>());
}
catch (const object_error& e) {
    std::cout << "error: " << e.what() << '\n';
}

mutate(hero, add<transform>(), add<health>(100));
```

```
error: world: mutate to object of type {'health'}: 'health' missing default init
```

```c++
// script_state is not copyable, so objects with it can't be copied
mutate(hero, add<script_state>());
try {
    auto copy = hero.copy();
}
catch (const object_error& e) {
    std::cout << "error: " << e.what() << '\n';
}
```

```
error: world: copy object of type {'transform', 'health', 'inventory', 'script'}: 'script' missing copy init
```

These checks happen at run time, because the composition of objects is only known at run time. A failed mutation leaves the object unchanged.

See [Mutations](mutations.md) for the ways to construct mixins with arguments.

## Names

Every mixin has a name. By default it's the name of the class as written in the macro (without namespaces): `transform`, `health`. It can be changed with `.name(str)`, like `script_state` which is named `"script"` above.

Names are used for diagnostics, like in the error messages above, and to refer to mixins by name:

```c++
// mixins can be added by name, too (with default construction)
mutate(hero, add("inventory"));
std::cout << "hero has inventory: " << hero.has("inventory") << '\n';
```

Working with names allows code which doesn't even have the declarations of mixins to compose objects. This is useful for things like serialization, scripting, configuration files, and plugins.

By default mixin names must be unique within a domain. Registering a mixin with a duplicate name is an error. This can be relaxed with `domain_settings::allow_duplicate_mixin_names`, but then operations by name become unreliable.

`.name(str)` doesn't copy the string. It must outlive the mixin's registration, which is always the case with string literals. Use `.store_name(str)` for names which are created at run time.

## Internal and external mixins

By default, all mixins of an object are allocated in a single buffer. Such mixins are called *internal*. They give objects good cache locality, and creating an object requires a single allocation.

When an object is mutated, a new buffer is allocated for the new type and internal mixins which remain in the object are *moved* to the new buffer. This is why internal mixins must be nothrow move constructible.

A mixin is *external* (allocated separately and never moved) when:

* it's not nothrow move constructible (like `script_state` above which has a `std::mutex`)
* it has a [mixin allocator](../advanced/allocators.md) (`.uses_allocator`)
* it's explicitly marked with `.force_external()`

> [!IMPORTANT]
> Since internal mixins are moved when an object is mutated, pointers and references to internal mixins are invalidated by mutations, much like pointers to elements of `std::vector` are invalidated when it grows. Pointers to external mixins remain valid as long as the mixin is part of the object. Use `.force_external()` for mixins which must have a stable address.

## Equality and comparison

`object::equals` and `object::compare` compare objects mixin by mixin. They only work if the mixins say how to compare them. This is never inferred automatically, and must be explicitly enabled in the definition:

* `.uses_equal()` uses the class's `operator==`, which must be `noexcept`. You can also provide a custom function: `.uses_equal(func)`.
* `.uses_compare(func)` sets a three-way comparison function which returns a negative number, zero, or a positive number. There is no default.

```c++
// transform uses operator== for equality checks
object a(g::get_domain<world>()), b(g::get_domain<world>());
mutate(a, add<transform>());
mutate(b, add<transform>());
std::cout << "equal: " << a.equals(b) << '\n'; // true
b.get<transform>()->y = 1;
std::cout << "equal: " << a.equals(b) << '\n'; // false
```

Objects of different types are never equal. With the same type:

* `equals` is true if each mixin compares equal with its equality function, or if it has none, with its comparison function. If a mixin has neither, the objects are not equal.
* `compare` uses the comparison functions of the mixins and throws `object_error` if a mixin doesn't have one. Objects of different types are compared by their types.

## Builder reference

The builder which `DYNAMIX_DEFINE_MIXIN` evaluates to is [`util::mixin_info_data_builder`](../../code/dynamix/mixin_info_data.hpp).

| Method | Description |
|---|---|
| `implements<Feature>(perks...)` | Implement a feature with the default payload for this mixin. For messages this binds them to methods. See [Messages](messages.md#implementing-messages). |
| `implements_by<Feature>(payload, perks...)` | Implement a feature with a payload which is referenced. For messages this is a function. |
| `implements_with<Feature>(payload, perks...)` | Implement a feature with a payload which is copied or moved and owned by the mixin's definition. |
| `name(str)` | Set the name. The string is not copied. |
| `store_name(str)` | Set the name and store a copy of the string. |
| `uses_equal(func = nullptr)` | Set the equality function. With no argument uses `operator==`. |
| `uses_compare(func)` | Set the comparison function. |
| `force_external(b = true)` | Make the mixin [external](#internal-and-external-mixins). |
| `order_priority(p)` | Set the key for the canonical order of mixins. See [Domains](domains.md). |
| `uses_allocator<Alloc>()`, `uses_allocator(alloc)` | Use a mixin allocator. Implies an external mixin. See [Allocators](../advanced/allocators.md). |
| `on_mutation(rule, perks)` | Add a common mutation rule for this mixin, like `also_adds<M>` or `attaches_to<M>`. See [Mutation Rules](mutation-rules.md). |
| `adds_mutation_rule(...)` | Add a custom mutation rule. Implies `dependency()`. See [Custom Mutation Rules](../advanced/mutation-rules.md). |
| `dependency(b = true)` | Mark the mixin as a dependency. Dependencies are removed at the start of every mutation and are expected to be re-added by mutation rules. See [Mutation Rules](mutation-rules.md). |
| `user_data(value)` | Set a `uintptr_t` value which the library doesn't touch. |
| `noop(anything)` | Do nothing. Useful in macros which extend mixin definitions. |

The perks for features are their [bid and priority](messages.md#bids-and-priorities).

For customizations which the builder doesn't cover, `DYNAMIX_DEFINE_MIXIN_EX(domain_tag, mixin, builder)` allows a custom builder class template, typically one derived from `mixin_info_data_builder`. And mixins can be defined without macros at all. See [Mixins Without Macros](../advanced/no-macros.md).

## Mixins in shared libraries

When a mixin is defined in a shared library and used from outside of it, its declaration must be exported. Use `DYNAMIX_DECLARE_EXPORTED_MIXIN` with your library's export macro:

```c++
DYNAMIX_DECLARE_EXPORTED_MIXIN(MYLIB_API, struct transform);
```

The definition is the same as for non-exported mixins.

Mixins which are defined in a plugin and are only referred to by name from the outside (or not at all) don't need to be exported. See [Multiple Domains, Shared Libraries, and Plugins](../advanced/plugins.md).
