# Comparisons: DynaMix vs X

Here's how DynaMix compares with existing solutions for dynamic polymorphism and object composition.

Most of these solutions share some ideas with DynaMix, and some were a direct inspiration for it. The point of this page is not to say that DynaMix is "better". The goal is to help you see where it fits, and which of its features you won't find elsewhere.

## Contents

* [C++ multiple inheritance](#c-multiple-inheritance)
* [Ruby mixins](#ruby-mixins)
* [Traits and CRTP mixins](#traits-and-crtp-mixins)
* [Type erasure libraries](#type-erasure-libraries)
* [COM](#com)
* [Entity Component Systems](#entity-component-systems)
* [Downsides](#downsides)

## C++ multiple inheritance

This is the closest thing to DynaMix in C++. It's covered in detail in the [overview](../overview.md#dynamix-vs-multiple-inheritance-in-c).

In short: DynaMix gives you the same kind of composition as multiple inheritance, but at run time. Types are composed when needed instead of being listed in code. Objects can change their type. Interfaces are separate from the classes which implement them. Overrides are decided by the object's composition, not by a class hierarchy.

## Ruby mixins

Ruby was a major inspiration for DynaMix. A Ruby module can be included into a class, or added to a single object with `Object#extend`. The second one is almost exactly what a DynaMix mutation does: it adds new behavior to an existing object at run time.

DynaMix adds some things which Ruby's mixins don't have:

* **Removing mixins.** A Ruby module can't be removed from an object once it's been extended with it. In DynaMix, mixins can be removed just as easily as they're added. Overridden implementations aren't lost. When the overriding mixin is removed, the object's previous behavior is restored. This is useful for status effects, states, temporary modes, and similar things.
* **Explicit control over overriding.** In Ruby the most recently added module wins. DynaMix also does this by default (see [overriding by order](../cpp/messages.md#by-order)), but it also offers [bids](../cpp/messages.md#by-bid). With a bid, a mixin can override others no matter when it was added. It can also guarantee that it won't override an existing implementation.
* **Multicasts.** A [multicast message](../cpp/messages.md#multicasts) executes the implementations of all mixins which implement it, in an order you control. In Ruby you need to build a chain of `super` calls for this, and every module in it needs to cooperate.
* **Reachable overrides.** Ruby has `super`. DynaMix has a counterpart: [calling the next implementation](../cpp/messages.md#calling-the-next-implementation).
* **Shared types.** A Ruby object extended with a module gets a singleton class of its own. A DynaMix type is shared by all objects with the same composition. Its feature table is computed once, so message calls stay fast no matter how objects got their composition.

And of course DynaMix is C++. That includes static typing for the arguments and return values of messages, value semantics, and [custom allocators](../advanced/allocators.md).

## Traits and CRTP mixins

Many languages have *traits* or *mixins* as a language feature: Scala, PHP, Rust, Self, Dart, Perl's roles, and others. In C++, the same thing is usually done with [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). In C++ circles *this* is what "mixin" usually means (see [Library name](../overview.md#library-name)).

All of these compose a type from reusable building blocks, which is what DynaMix does too. The difference is *when*. Traits are a compile-time tool, and this has some consequences:

* **Static types.** A type composed of traits is fixed. An object can't gain or lose a trait.
* **Combinatorial explosion.** Every combination of traits has to be written in code. With *N* optional building blocks you could have 2<sup>*N*</sup> types. In practice you can only write a few of them.
* **No common interface.** With CRTP the composed types have no common base. To handle them in a uniform way you need templates, which propagate to the code that uses them, or a separate layer of virtual interfaces. Some languages (Rust's `dyn Trait`, for example) offer dynamic dispatch for traits, but only one trait (interface) at a time, and only for types which are known in advance to implement it.
* **Compilation dependencies.** The code which composes a type must see the definitions of all of its building blocks.

DynaMix has none of these problems. The building blocks are combined at run time, all objects are `dynamix::object`, and code which uses an object only needs to see the declarations of the messages it calls. It doesn't need to see a single mixin.

What traits do better is that they're free. Everything is resolved at compile time, so they cost nothing at run time. If your types are known at compile time and don't change, traits or CRTP are a better choice.

## Type erasure libraries

There are many C++ libraries for type erasure: [Boost.TypeErasure](https://www.boost.org/doc/libs/release/doc/html/boost_typeerasure.html), [Dyno](https://github.com/ldionne/dyno), [Proxy](https://github.com/microsoft/proxy), and others. The standard library also has some special cases of type erasure: `std::function`, `std::any`, and `std::move_only_function`.

They let you have value types with polymorphic behavior without inheritance. A type implements an interface just by having the right methods, and it doesn't need a common base. DynaMix shares this *non-intrusiveness*. Mixins are plain classes too, and they don't need to know about DynaMix or about each other.

The difference is in what they wrap. A type erasure wrapper holds a single concrete object, and the interface is defined in advance as a fixed set of operations. DynaMix objects are made of many mixins, which provide many features, and both can change at run time:

* An object's interface is the union of the features of its mixins. A single interface may be implemented by many different mixins.
* The set of features isn't closed. New messages can be declared and implemented anywhere, even in [plugins](../advanced/plugins.md), without changing existing code.
* There are no type erasure counterparts for multicasts, overriding, and [reflection](../advanced/reflection.md).

If you need one polymorphic interface for one value, a type erasure library is simpler and likely faster. If your objects are made of many parts which provide many behaviors, DynaMix is a better fit.

## COM

[COM](https://en.wikipedia.org/wiki/Component_Object_Model) is the most widely used system with goals similar to those of DynaMix. It provides a dynamic object model for C and C++ which isn't tied to the language's own type system. A COM object implements interfaces, and its users discover them at run time with `QueryInterface`.

The two are similar in that objects are composed of parts which provide abstract interfaces. Users don't need to know the concrete composition. Here's how they differ:

* **No interfaces as such.** A COM interface is a fixed set of methods which an object implements entirely or not at all. In DynaMix every message is independent. A mixin can implement any combination of messages, and a group of messages can be implemented by several mixins.
* **Mutation.** The interfaces of a COM object are determined by its implementation. It can't gain or lose them during its lifetime. DynaMix objects can be mutated at any time, and removing a mixin restores the previous state of the object.
* **Multicasts and overriding.** COM has no counterpart of multicasts, bids, and priorities.
* **Idiomatic C++ objects.** COM objects are reference counted and accessed through pointers. You release them by calling `Release`, not by destroying them. DynaMix objects are regular C++ objects: they have destructors, they can be moved, and they can be [copied explicitly](../cpp/objects.md#copying). Mixins are regular C++ classes. They don't need to be derived from `IUnknown`, and they don't need IDL or generated code.
* **Different scope.** COM is also a binary interface standard: it supports communication between processes and between machines, and it's language-agnostic. DynaMix does none of this. It's a library for organizing code in a single process. It can be used from [C](../c/c-interface.md) and in [shared libraries and plugins](../advanced/plugins.md), but it doesn't try to be a cross-language or cross-process component model.

## Entity Component Systems

This comparison comes up a lot, especially in game development. There's often confusion about it, because "entity component system" is used for two very different things.

### The entity-component pattern

The older meaning (sometimes called *component-based architecture*) is a pattern for composing objects from components. An entity is a container of components. Behavior is added by adding a component, and code finds a component with something like `entity.getComponent<Physics>()`. Many game engines work this way.

DynaMix is the same idea in its structure: `dynamix::object` is the entity, and mixins are the components. The difference is that DynaMix makes the composition *polymorphic*:

* **Users call features, not components.** With the entity-component pattern, code has to know which component provides a behavior. Then it gets that component and calls a method of it. With DynaMix, code calls a message for the object (`render(obj)`) and doesn't know or care which mixin implements it. Different objects can implement it with different mixins. This is the main point of the library.
* **Overriding and multicasts.** A mixin can override a behavior provided by another, and when it's removed the old behavior comes back. A multicast executes the implementations of all mixins which provide it. With the entity-component pattern, the components or the code that uses them have to coordinate all of this by hand, often through events or messaging systems.
* **Non-intrusive.** Mixins don't need a common base class like `Component`, and they don't need virtual functions.
* **Types are first-class.** In DynaMix every unique composition is a [type](../cpp/types-and-mutations.md#types). Types can be queried, categorized with [type classes](../cpp/type-classes.md), and reused to create objects. Lookups of mixins and features in a type are O(1).

There are also some limits which DynaMix has and some entity-component designs don't. A mixin instance belongs to exactly one object. It can't be shared between several objects. An object can have at most one instance of a given mixin.

If you have used or considered the entity-component pattern, DynaMix will probably feel familiar, and it can likely replace it.

### Data-oriented ECS

The more modern meaning is the *data-oriented* ECS ([EnTT](https://github.com/skypjack/entt), [flecs](https://github.com/SanderMertens/flecs), Unity's DOTS, and others). There, entities are just IDs, components are plain data stored in contiguous arrays, and *systems* are functions which iterate over all entities with a given set of components.

The goal of this kind of ECS is performance through cache locality and batch processing. In it, there's no per-entity polymorphism by design. A system does the same thing to all the matching data.

DynaMix has a different goal and works differently:

* **Object-oriented vs data-oriented.** DynaMix is about objects and calls polymorphic behavior on a single object. A data-oriented ECS is about data and processes many entities at once with no polymorphic calls at all.
* **Memory layout.** The mixins of a DynaMix object are stored together (by default in a single buffer per object, see [Allocators](../advanced/allocators.md)). A data-oriented ECS stores the components of each type together for all entities.
* **Performance.** A polymorphic call has a cost and will often cause a cache miss. The cost of a DynaMix message call is comparable to that of `std::function`. This is fine for most code, but not for a loop over millions of entities in every frame.

So DynaMix is not an alternative to a data-oriented ECS. It can't help with what the ECS is for. The two solve different problems, and they work together well. A data-oriented ECS can handle the performance-critical parts (physics, rendering, particles), and DynaMix can handle the complex and diverse logic (gameplay, AI, UI, tools) in the same program. This is the same advice as in [When (and when not) to use DynaMix](../overview.md#when-and-when-not-to-use-dynamix).

## Downsides

Compared to the static solutions above (multiple inheritance, traits, CRTP), DynaMix has a price:

* **No compile-time checks for features.** Since an object's composition is only known at run time, calling a message which the object doesn't implement is a run-time error (`dynamix::feature_error`), not a compile-time one. Such errors are harder to catch. [Default implementations](../cpp/messages.md#default-implementations), [type classes](../cpp/type-classes.md), and [querying](../cpp/messages.md#querying) can help.
* **Run-time cost.** Message calls cost about as much as a `std::function` call, more than a regular virtual call. Mutations allocate and move mixins.
* **Memory overhead.** Each unique type has metadata, which includes its feature table. Each object has a pointer to its type and a buffer for its mixins. Each mixin is preceded by a pointer to its owning object.

See [Performance](../working-with/perf.md) for more details.
