# Types and Mutations

A mutation changes the composition of an object: it adds, removes, or reorders mixins. Since the composition of an object defines its type, a mutation changes the object's type, and with it the object's [features](messages.md): what it can do and how it does it.

This is the core difference between DynaMix and C++ inheritance. A C++ object has the same type for its entire lifetime. A DynaMix object can change its type at any time, while it remains the same object. All references and pointers to the object stay valid, and code which works with the object through its features doesn't need to know that anything happened.

> [!TIP]
> The code on this page is from [example/guide/e-mutations.cpp](../../example/guide/e-mutations.cpp).

## Contents

* [Mutating objects](#mutating-objects)
* [Constructing mixins](#constructing-mixins)
* [Mutation ops](#mutation-ops)
* [What happens in a mutation](#what-happens-in-a-mutation)
* [Errors and exception safety](#errors-and-exception-safety)
* [Types](#types)
* [Gradual mutations](#gradual-mutations)
* [Sealed objects](#sealed-objects)

## Mutating objects

```c++
#include <dynamix/mutate.hpp>

object obj(dom);
mutate(obj, add<transform>(), add<health>(100));
print_type(obj); // {transform, health}

mutate(obj, remove<transform>(), add<ai>("patrol.lua"));
print_type(obj); // {health, ai}
```

`dynamix::mutate(obj, ops...)` applies a list of *mutation ops* to an object in a single step. No matter how many ops there are, the object goes directly from its current type to the resulting one. Mixins which are in both types are preserved, mixins which are only in the new type are constructed, and mixins which are only in the old type are destroyed.

New mixins are added at the back of the type. The order of mixins matters: it decides which implementations [override](messages.md#by-order) others and the [order of multicasts](messages.md#execution-order).

## Constructing mixins

New mixins are constructed in one of three ways:

* With their default constructor: `add<transform>()`. If a mixin has no default constructor, this throws `object_error` (see [Defining Mixins](mixins.md#what-dynamix-infers-from-the-class)).
* With constructor arguments: `add<health>(100)`, `add<ai>("patrol.lua")`.
* With a function, which receives an `init_new_args` and must construct the mixin with placement new into `args.mixin_buf`:

```c++
// the construction of a mixin can be customized by a function
mutate(obj, add(g::get_mixin_info<ai>(), [](init_new_args args) {
    new (args.mixin_buf) ai("custom.lua");
}));
```

`init_new_args` also contains the mixin info, the target type, and the mixin's index in it. The function must either construct the mixin or throw an exception. Returning without constructing it leads to undefined behavior.

## Mutation ops

All ops are available in the namespace `dynamix` (and `dynamix::mutate_ops`).

| Op | Description |
|---|---|
| `add<M>(args...)` | Add `M`, constructed with `args...` (default constructed if there are none). |
| `add(info)`, `add(name)` | Add a mixin by `mixin_info` or by name, default constructed. |
| `add(info, func)`, `add(name, func)` | Add a mixin by info or by name, constructed by `func`. |
| `add_if_lacking<M>(args...)` | Add `M` only if the object doesn't already have it. `args...` are only used if the mixin is constructed. |
| `add_if_lacking(info)`, `add_if_lacking(name)` | Add a mixin by info or by name only if the object doesn't already have it. |
| `remove<M>()`, `remove(info)`, `remove(name)` | Remove a mixin. Removing a mixin which the object doesn't have does nothing. |
| `to_back<M>()`, `to_back(info)`, `to_back(name)` | Move a mixin to the back of the type. |

Ops are applied in the order in which they're listed. Thus `mutate(obj, remove<a>(), add<a>())` is equivalent to `to_back<a>()`: the result is a type in which `a` is at the back, and since it's present both in the old and in the new type, the existing instance of `a` is preserved. A mixin is never destroyed and reconstructed in a single mutation.

```c++
mutate(obj, to_back<health>());
print_type(obj); // {transform, ai, health}
```

> [!NOTE]
> If the domain canonicalizes types (`domain_settings::canonicalize_types`), the order of mixins doesn't depend on the order of the ops, and `to_back` has no effect.

### `add_if_lacking` and arguments

`add_if_lacking` is useful when you need a mixin to be present, but don't know whether it already is. The arguments for its constructor are only used if it's actually constructed. If the object already has the mixin, the arguments are not touched, even if they're passed with `std::move`:

```c++
std::string script = "guard.lua";
mutate(obj, add_if_lacking<ai>(std::move(script)));
// obj already has ai, so script was not moved from
std::cout << "script: " << script << '\n'; // script: guard.lua
```

To achieve this, `add_if_lacking` holds references to its arguments. This is safe as long as it's used inside the `mutate` call, as above. Don't store the result of `add_if_lacking` (or any other op) in a variable to use it later.

### `remove` by name and `using namespace`

> [!WARNING]
> Always qualify remove by name: `dynamix::remove("name")`.

The C standard library has a function `::remove(const char*)` which *deletes a file*. With `using namespace dynamix;`, an unqualified `remove("name")` would be an exact match for it, and would silently be preferred over `dynamix::remove(std::string_view)`. To prevent this, either fully qualify it (`dynamix::remove("name")`) or explicitly convert the argument to `std::string_view`: `remove(std::string_view("name"))`.

`remove<M>()` and `remove(info)` don't have this problem, and neither does `remove("name")` in [gradual mutations](#gradual-mutations), where it's a member function.

## What happens in a mutation

1. The ops are applied to a [`type_mutation`](../../code/dynamix/type_mutation.hpp) which starts from the object's current type.
2. The domain gets the type for the result. This includes applying [mutation rules](mutation-rules.md), which may add or remove more mixins. Mixins which are marked as [dependencies](mutation-rules.md) are removed before the rules are applied, and it's up to the rules to add them back. Types are cached, so this step is a quick lookup for mutations which have already been seen.
3. If the resulting type is the same as the object's current one, nothing happens.
4. The new mixins are constructed in the order of the new type. Each is constructed with its op, if one was provided, or with its default constructor otherwise. Mixins which were added by mutation rules are default constructed.
5. The mutation is finalized: mixins which are only in the old type are destroyed, and [internal](mixins.md#internal-and-external-mixins) mixins which are in both types are moved to the new object buffer.

There are some consequences:

* While a new mixin is being constructed, the object still has its old type. See [Dangerous Functionalities](../advanced/danger.md#accessing-the-object-from-a-mixins-constructor-or-destructor).
* Mutations invalidate pointers and references to internal mixins, even to the ones which remain in the object.
* Mutations don't use mixins' copy or move assignment, and moved-out-from internal mixins are destroyed.

## Errors and exception safety

Mutations have a strong exception guarantee: if anything throws, the object is unchanged. This includes mixin constructors:

```c++
// if a mixin constructor throws, the object is not changed
auto ai_ptr = obj.get<ai>();
try {
    mutate(obj, remove<ai>(), add<cursed>(true));
}
catch (const std::runtime_error& e) {
    std::cout << "error: " << e.what() << '\n';
}
print_type(obj); // {transform, ai, health}
std::cout << "same ai: " << (obj.get<ai>() == ai_ptr) << '\n'; // true
```

(This is possible since new mixins are constructed before anything happens to the old ones. If a constructor throws, the new mixins which were already constructed are destroyed and the object is left as it was.)

Errors in the mutation itself throw `dynamix::type_error`:

```c++
// adding a mixin which is already in the object is an error
mutate(obj, add<ai>());
// error: game: creating type {'transform', 'ai', 'health', 'ai'}: 'ai' duplicate

// so is referring to a mixin by a name which doesn't exist
mutate(obj, add("flying"));
// error: game: creating type {'transform', 'ai', 'health'}: unknown mixin 'flying' in add
```

Other errors are:

* A new mixin which has to be default constructed, but is not default constructible: `object_error`.
* A feature [clash](messages.md#clashes): `type_error`.
* Mutating a [sealed](#sealed-objects) object: `object_error`.

## Types

A type is a list of mixins, together with the information needed to create objects of it and to look up their features. Types are created by domains and are cached. Objects with the same mixins in the same order share the same type instance:

```c++
// objects with the same mixins share the same type
object other(dom);
mutate(other, add<transform>(), add<ai>(), add<health>(5));
std::cout << "same type: " << (&obj.get_type() == &other.get_type()) << '\n'; // true
```

Types can be created without objects, through a `type_mutation`, or with `domain::get_type` with a list of mixin infos. Objects can then be created from a type, or mutated to it:

```c++
// types can be created without objects
type_mutation tmut(dom);
tmut.add<transform>();
tmut.add<ai>();
const type& t = dom.get_type(std::move(tmut));

// and objects can be created from types (with default construction)
object npc(t);
print_type(npc); // {transform, ai}

// or mutated to a type, optionally constructing some of the mixins with arguments
mutate_to(npc, other.get_type(), construct<health>(30));
print_type(npc); // {transform, ai, health}
```

`mutate_to(obj, type, ops...)` takes `construct<M>(args...)` and `construct(info_or_name, func)` ops for the new mixins of the type. Mixins without an op are default constructed. Mutation rules are not applied, since the type is already known.

This is useful when many objects of the same type need to be created. The type is created once and creating each object doesn't need to look it up or apply mutation rules. It's also a way to implement prototypes, archetypes, or object templates.

A `type` also has queries which don't need an object: `has<M>()`, `index_of`, `implements<Feature>()`, [type classes](type-classes.md), and more. A `type_mutation` supports the same queries on a type which is not yet created, which is useful in [mutation rules](mutation-rules.md).

### Lifetime of types

Types are owned by their domain. A type which is no longer used by any object is not destroyed automatically. It stays in the domain and is reused when an object with the same composition is needed again. `domain::garbage_collect_types()` destroys all types which have no objects.

References to types are valid until the type is destroyed. If your code may call `garbage_collect_types`, don't hold references to types that may have no objects.

## Gradual mutations

There's an alternative syntax for mutations, which is compatible with DynaMix v1:

```c++
// a simpler mutation for mixins which don't need constructor arguments
// it's applied when the returned value is destroyed (at the semicolon)
mutate(npc)
    .remove<ai>()
    .add_if_lacking<transform>()
    .add<ai>();
print_type(npc); // {transform, health, ai}
```

`mutate(obj)` with no ops returns a `gradual_mutation` which collects ops and applies them in its destructor. It supports `add`, `add_if_lacking`, `remove`, and `to_back` by type, info, or name. It can also be applied explicitly with `apply()` or cancelled with `cancel()`.

It's less capable than the variadic `mutate`: all new mixins are default constructed. On the other hand it's simpler and involves less template code, so it's noticeably faster to compile. For code with many mutations which only add and remove default-constructible mixins, it may be the better choice.

Since it's applied in a destructor, a gradual mutation which fails throws from a destructor. This is fine in a single statement like the one above. If you store it in a named variable, note that it's *not* applied if the scope is exited because of an exception.

## Sealed objects

`object::seal()` makes an object's composition immutable: any attempt to mutate it, clear it, or assign to it throws `object_error`. Sealing is useful for objects whose composition must not change after they're set up. It can't be undone. It is transferred when the object is moved, but not to copies. Objects can also be created sealed: `object(type, alloc, true)`.

## Advanced

* [`object_mutation`](../../code/dynamix/object_mutation.hpp) is the low-level class which all mutations use. It allows piecewise mutations with full control over how each mixin is constructed or updated.
* Custom ops can be created by deriving from `object_mutate_op`, or `common_mixin_init<M>` for ops which construct a specific mixin.