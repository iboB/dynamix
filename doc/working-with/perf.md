# Performance

DynaMix is a tool for dynamic polymorphism, and like all dynamic polymorphism (virtual functions, `std::function`, type erasure) it has a cost. This page explains what the costs are, where they come from, and how to keep them low.

In short: message calls cost a bit more than virtual calls, objects use slightly more memory than the sum of their parts, and mutations are relatively expensive. Mutations are meant to happen when an object's behavior changes, not in a hot loop.

> [!TIP]
> All numbers on this page come from the benchmarks in [bench/](../../bench/). You can run them yourself. Results on your hardware will differ.

## Contents

* [Summary](#summary)
* [How the numbers were measured](#how-the-numbers-were-measured)
* [Message calls](#message-calls)
* [Objects](#objects)
* [Mutations](#mutations)
* [Types](#types)
* [Multiple threads](#multiple-threads)
* [Guidelines](#guidelines)
* [Regression testing](#regression-testing)

## Summary

| Operation | Typical cost | For comparison |
|-----------|--------------|----------------|
| Unicast message call | 7-8 ns, about 50 instructions | Virtual call: 5-6 ns, about 19 instructions |
| `has<M>()`, `implements<Msg>()`, type class check | about 1 ns | |
| `get<M>()` | about 5 ns | |
| Creating an object from a type (5 mixins) | about 50 ns | C++ class: 11 ns. Entity with 5 components: 35 ns |
| Copying or destroying an object | 20-50 ns | Entity with 5 components: about the same |
| A mutation of an object (existing type) | about 85 ns | Adding a component to an entity: about 17 ns |
| Creating a new type | 0.4-3 µs | |
| Memory per mixin in an object | 24 bytes + the mixin itself | |
| Memory per type | 0.3-3 KB | |

## How the numbers were measured

The times on this page were measured on an Intel Core Ultra 9 285K (pinned to a performance core), Ubuntu 24.04, GCC 13, in a release build, with DynaMix as a shared library. Instruction counts come from [cachegrind](#regression-testing).

Times are in nanoseconds per operation, and are the fastest of 10 samples. Treat them as approximate: what matters is the order of magnitude and the comparison with the alternatives, not the exact values.

Most benchmarks compare DynaMix with C++ alternatives which do the same thing:

* **Virtual functions** and **`std::function`** for calls.
* **A C++ class** which has all components as members. This is the fastest possible option, but it's not dynamic at all.
* **The entity-component pattern**: an entity with a vector of individually allocated polymorphic components. This is the closest C++ alternative to DynaMix objects. See [DynaMix vs X](../misc/dynamix-vs-x.md#entity-component-systems).

## Message calls

A unicast message call does the following:

1. Gets the type of the object.
2. Looks up the message in the type's feature table. This is constant-time and doesn't depend on how many mixins or features the type has.
3. Gets the mixin from the object.
4. Calls the implementation through a function pointer.

A virtual call has a similar indirect call, but fewer memory accesses before it.

### Unicasts

Objects with a single mixin implementing a message, compared with virtual functions and `std::function` ([msg-unicast](../../bench/msg-unicast/)):

| | Virtual | `std::function` | Message |
|-|--------:|----------------:|--------:|
| Sequential access, 1M objects | 6 ns | 5 ns | 8 ns |
| Shuffled access, 1K objects | 5 ns | 4 ns | 7 ns |
| Shuffled access, 1M objects | 14 ns | 10 ns | 26 ns |
| Instructions per call | 19 | 22 | 50 |

When the objects are in the cache, a message call is about 1.25 times slower than a virtual call. When objects are accessed in an order which doesn't match their placement in memory, and there are too many of them to fit in the cache, cache misses dominate. In this case a message call is about 1.9 times slower than a virtual call. This is because a message call accesses more memory: the object, its type, and the mixin.

### Multicasts

Objects with 2 to 6 mixins which implement a multicast, compared with vectors of virtual objects or `std::function`-s ([msg-multicast](../../bench/msg-multicast/)):

| | Virtual | `std::function` | Message |
|-|--------:|----------------:|--------:|
| Shuffled access, 1K objects | 33 ns | 28 ns | 35 ns |
| Shuffled access, 1M objects | 106 ns | 87 ns | 117 ns |
| Instructions per call | 71 | 80 | 107 |

A multicast costs about as much as calling the same functions through a vector of polymorphic objects. The mixins of an object are next to each other in memory, which offsets the cost of the more complex call.

### Kinds of calls

Objects with 10 mixins, where the called one is in the middle ([msg-call](../../bench/msg-call/)). These are instruction counts per call, as the times are too small to measure precisely:

| Call | Instructions |
|------|-------------:|
| Direct (non-virtual) call | 12 |
| Virtual call | 13 |
| Message | 45 |
| `obj.get<M>()->method()` | 67 |
| Message whose implementation calls the next one (`DYNAMIX_CALL_NEXT_IMPL_MSG`) | 109 |
| Message with a default implementation, not implemented by any mixin | 37 |

Some things to note:

* Calling a message takes fewer instructions than getting a mixin and calling its method. Calling messages isn't just more flexible, it's also at least as fast.
* Calling the next implementation costs about as much as another message call.
* The position of the implementing mixin in the type and the number of mixins and features don't matter.

### Queries

The same objects, half of which have a mixin with a message ([msg-call](../../bench/msg-call/)):

| Query | Time | Instructions |
|-------|-----:|-------------:|
| `obj.has<M>()` | 1 ns | 37 |
| `obj.get<M>()` | 5 ns | 57 |
| `obj.has("name")` | 7 ns | 178 |
| `obj.get_type().implements_strong<Msg>()` | < 1 ns | 34 |
| `obj.is_of<TypeClass>()` | 1 ns | 25 |

Queries by name are linear: they compare the name to the names of all mixins in the type. Prefer queries by type, info, or id in performance-critical code. Type class checks are not cached, so their cost depends on the match function. Here it's a single `has` check.

## Objects

### Memory

An object is a `dynamix::object`, which is 32 bytes on 64-bit platforms, and a buffer for its mixins. The buffer is allocated with the object's [allocator](../advanced/allocators.md). In addition to the mixins themselves, the buffer has per-mixin data, including a pointer to the object before each mixin (which is how [`object_of`](../cpp/mixins.md#object_of) works).

From the [memory report](../../bench/memory/):

| Mixins | External mixins | Mixin data | Allocated | Allocations |
|-------:|----------------:|-----------:|----------:|------------:|
| 1 | 0 | 8 B | 32 B | 1 |
| 5 | 0 | 40 B | 160 B | 1 |
| 10 | 0 | 80 B | 320 B | 1 |
| 5 | 1 | 40 B | 160 B | 2 |
| 5 | 5 | 40 B | 160 B | 6 |

So the overhead is 24 bytes per mixin, plus padding for alignment. An object with only internal mixins makes a single allocation. Every external mixin makes one more.

For comparison, an entity with polymorphic components has an allocation per component, and each component has at least a pointer to its vtable. A vector of pointers to them is another allocation.

### Creating, copying, and destroying

64K objects with 5 mixins: two with 3 doubles, one with 2 ints, one with a `std::string`, and one with a `std::vector` ([object-lifetime](../../bench/object-lifetime/)):

| | C++ class | Entity | DynaMix |
|-|----------:|-------:|--------:|
| Create (from a type) | 11 ns | 35 ns | 50 ns |
| Create (from a type, arena allocator) | | | 43 ns |
| Create (empty object + `mutate`) | | | 106 ns |
| Copy | 7 ns | 49 ns | 47 ns |
| Destroy | 3 ns | 21 ns | 20 ns |
| Destroy (arena allocator) | | | 15 ns |

Copying and destroying DynaMix objects costs about as much as for entities with components. Creating them from a type costs more. Creating an empty object and mutating it costs twice as much as creating it from a type, since it includes a mutation (see below).

## Mutations

A mutation of an object with an existing type:

1. Applies the mutation ops to a `type_mutation`.
2. Looks up the resulting type in the domain.
3. Allocates a new buffer and constructs the new mixins.
4. Moves the internal mixins which remain to the new buffer, destroys the removed mixins, and deallocates the old buffer.

(See [What happens in a mutation](../cpp/types-and-mutations.md#what-happens-in-a-mutation).)

Objects with 4 mixins, to which a fifth is added and then removed ([object-lifetime](../../bench/object-lifetime/)):

| | Time | Instructions |
|-|-----:|-------------:|
| Entity with components: add and remove a component | 35 ns | 1388 |
| DynaMix: add and remove a mixin (two mutations) | 173 ns | 4022 |
| DynaMix with a recycling allocator | 168 ns | 3706 |

A mutation costs about 85 ns, which is about 5 times more than adding or removing a component of an entity. The allocations are not the bottleneck: an allocator which reuses buffers barely changes the result. Most of the time is spent creating the mutation and looking up the type (see [Types](#types)).

Each mutation makes one allocation and one deallocation, plus one allocation per added external mixin.

### Internal and external mixins

Since internal mixins are moved on every mutation, one might expect big internal mixins to make mutations expensive. Objects with a 512-byte mixin, to which a small mixin is added and then removed:

| 512-byte mixin | Time (two mutations) |
|----------------|-----:|
| Internal | 155 ns |
| External | 147 ns |

The difference is small. Moving a few hundred bytes is cheap compared to the rest of the mutation. Make mixins external when they need a stable address, or when they're very big or expensive to move, but don't expect a big speed-up otherwise. See [Internal and external mixins](../advanced/allocators.md#internal-and-external-mixins).

## Types

Types are created when a mutation leads to a composition which the domain hasn't seen before. They're cached, and then looked up on every following mutation ([type-creation](../../bench/type-creation/)):

| Types with 5 mixins | Create | Look up (1K types) | Look up (16K types) |
|---------------------|-------:|-------------------:|--------------------:|
| No features | 0.45 µs | 73 ns | 154 ns |
| 10 mutation rules in the domain | 0.73 µs | 75 ns | 153 ns |
| 20 features per mixin | 1.4 µs | 73 ns | 171 ns |

Creating a type includes applying the mutation rules and building the feature table, so it gets more expensive with more rules and features. This happens only once per unique type.

Looking up an existing type doesn't depend on the features or the rules, but it does depend on the number of types in the domain. With thousands of types it's noticeably slower.

Memory allocated by the domain for a new type, including its entry in the type cache ([memory report](../../bench/memory/)):

| Mixins | Features per mixin | Bytes per type |
|-------:|-------------------:|---------------:|
| 2 | 0 | 313 |
| 10 | 0 | 498 |
| 2 | 5 | 1596 |
| 10 | 5 | 3109 |

Types which no longer have any objects are not destroyed automatically. If a program creates many short-lived compositions, call `domain::garbage_collect_types()` from time to time. See [Lifetime of types](../cpp/types-and-mutations.md#lifetime-of-types).

## Multiple threads

With 4 threads on 4 cores, each working on its own part of the same data ([threads](../../bench/threads/)):

| | 1 thread | 4 threads | Speed-up |
|-|---------:|----------:|---------:|
| Message calls (1M objects) | 9 ns | 4 ns | 2.2x |
| Virtual calls (1M objects) | 11 ns | 5 ns | 2.2x |
| Mutations of separate objects | 166 ns | 176 ns | none |
| Adding and removing components of separate entities | 17 ns | 7 ns | 2.4x |
| Creating types in the same domain | 0.8 µs | 2.1 µs | slower |

Message calls don't modify anything and scale like virtual calls.

Mutations don't scale at all. Every mutation looks up a type in its domain, which takes a shared lock of the domain's type registry. The most likely reason is that, although the lock is shared, taking it still modifies its state, so all threads write to the same memory and end up waiting for each other. This is a known issue, which we intend to improve.

Creating types takes an exclusive lock, so it doesn't scale either. Creating them in several threads at the same time is even slower than creating them in one.

## Guidelines

* **Use messages for behavior.** They're the most flexible way to call functionality of a mixin, and they're at least as fast as `get<M>()` followed by a method call.
* **Don't mutate objects in hot loops.** Mutations are meant to change the behavior of an object: when it gets a status effect, enters a new state, and so on. If something changes every frame, keep it in the data of a mixin.
* **Create objects from types.** If you create many objects with the same composition, get the type once and create objects from it (`object(type)` or [`mutate_to`](../cpp/types-and-mutations.md#types)). This is twice as fast as creating an empty object and mutating it.
* **Combine mutations.** Every `mutate` call looks up a type, allocates a new buffer, and moves the mixins to it, regardless of how many ops it has. A single `mutate` with many ops goes directly to the resulting type, while many calls with a single op each repeat this work.
* **Avoid queries by name in performance-critical code.** They're linear. Queries by type, info, or id are constant-time.
* **Use allocators for objects which live and die together.** An arena allocator makes creating and destroying objects cheaper, and keeps the objects close together in memory. See [Allocators](../advanced/allocators.md).
* **Keep the number of types in check.** Type lookups get slower with more types. Avoid generating huge numbers of unique compositions, and garbage-collect types which are no longer used.
* **Prefer one thread for mutations of objects in the same domain.** Calls scale with threads. Mutations currently don't.
* **Know when not to use DynaMix.** Code which processes many similar objects in a tight loop, like physics or particles, is better served by data-oriented code with no polymorphism at all. See [When (and when not) to use DynaMix](../overview.md#when-and-when-not-to-use-dynamix).

## Regression testing

Timings on CI machines are too noisy to detect performance regressions. Instead, CI runs the single-threaded benchmarks with cachegrind and compares the number of executed instructions with a baseline. An increase of more than 2% in any benchmark fails the check. See [bench/cachegrind](../../bench/cachegrind/README.md).
