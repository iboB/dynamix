# Reflection and Introspection

Since DynaMix types are created at run time, everything about them is available at run time: which mixins a type has, which features it implements, which mixins implement them, and how. Domains know all of their mixins, features, types, rules, and type classes. Mixins and features have names and can be referred to by name. Together, these give complete runtime reflection of DynaMix objects, which plain C++ classes don't have.

> [!TIP]
> The code on this page is from [example/guide/reflection.cpp](../../example/guide/e-reflection.cpp).

## Contents

* [Everything in a domain](#everything-in-a-domain)
* [Working by name](#working-by-name)
* [Inspecting objects and types](#inspecting-objects-and-types)
* [Debug dumps](#debug-dumps)

## Everything in a domain

[`dynamix::domain_traverse`](../../code/dynamix/domain_traverse.hpp) iterates over the registries of a domain:

```c++
// everything that's registered in the domain
// (the traverse object locks the domain, so it's in a scope)
{
    domain_traverse trav(dom);
    trav.traverse_mixins([](const mixin_info& m) {
        std::cout << "mixin '" << m.name.to_std() << "' implements:";
        for (auto& f : m.features_span()) {
            std::cout << ' ' << f.info->name.to_std();
        }
        std::cout << '\n';
    });
}
```

```
mixin 'transform' implements:
mixin 'mesh' implements: render_msg
mixin 'ai' implements: update_msg
```

It has `traverse_mixins`, `traverse_features`, `traverse_types`, `traverse_type_queries`, `traverse_mutation_rules`, and `traverse_type_classes`.

> [!WARNING]
> A `domain_traverse` holds a (shared) lock on the domain for its *entire lifetime*, not only while traversing. While one exists, operations which modify the domain, like registering mixins or creating new types (which mutations may do), will wait for it to be destroyed. In the same thread this is a deadlock. Keep `domain_traverse` objects in a short scope, and don't mutate objects or register anything in its callbacks.

A [`mixin_info`](../../code/dnmx/mixin_info.h) has the mixin's name, size and alignment, its features with their bids and priorities, and more. A [`feature_info`](../../code/dnmx/feature_info.h) has the feature's name and its default payload.

## Working by name

Mixins, features, and type classes can be referred to by name:

* `domain::get_mixin_info(name)`, `get_feature_info(name)`, `get_type_class(name)`
* `add(name)`, `dynamix::remove(name)`, `to_back(name)`, `add_if_lacking(name)` in mutations
* `has(name)`, `get(name)` on objects, and `has(name)`, `index_of(name)` on types
* `implements_strong(name)` on types
* `is_of(name)` on objects and types

This allows working with objects without having the declarations of their mixins, which is useful for serialization, scripting, editors and tools, and data-driven composition:

```c++
// compose an object from names, for example read from a file
std::vector<std::string> composition = {"transform", "mesh", "ai"};
object obj(dom);
for (auto& name : composition) {
    mutate(obj, add(name));
}
```

Lookups by name are slower than by type or by info: they involve string comparisons. For performance-critical code, look up the info once and use it afterwards.

## Inspecting objects and types

A [`type`](../../code/dynamix/type.hpp) has the list of its mixins, the indices of mixins by id, and its feature table:

```c++
// inspect an object
const type& t = obj.get_type();
for (mixin_index_t i = 0; i < t.num_mixins(); ++i) {
    std::cout << i << ": " << t.mixins[i]->name.to_std() << '\n';
}
std::cout << "implements render: " << t.implements_strong("render_msg") << '\n';
std::cout << "implements update: " << t.implements_strong("update_msg") << '\n';

// which mixin implements update?
auto entry = t.ftable_at(update_msg::info.id);
std::cout << "update is implemented by: " << t.mixins[entry.begin->mixin_index]->name.to_std() << '\n';
```

```
0: transform
1: mesh
2: ai
implements render: true
implements update: true
update is implemented by: ai
```

The feature table is described in [Custom Features](custom-features.md#accessing-the-feature). Types also have queries about their capabilities, like `default_constructible()`, `copyable()`, and `comparable()`, and the number of objects which currently have them: `num_objects()`.

All of this works without objects. [Type-level polymorphism](../overview.md#dynamix-vs-multiple-inheritance-in-c), checking what objects of a type would be able to do before creating any, is something C++ classes can't offer.

## Debug dumps

[`dynamix::util::dbg_dmp`](../../code/dynamix/dbg_dmp.hpp) writes a human-readable description of a domain, a type, a mixin info, a feature info, a mutation rule, a type class, or an object to a `std::ostream`:

```c++
util::dbg_dmp(std::cout, obj);
```

```
object (0x7fff1e6d40d0)
  type: {'transform', 'mesh', 'ai'} (0x5597853d44c0) ('game')
    'transform': 0x5597853d4678
    'mesh': 0x5597853d4688
    'ai': 0x5597853d4698
```

Most overloads take flags which select what to include. The flags are in the `dnmx_dbg_dmp_flags` enum. By default everything is included. Dumping a whole domain is useful in debugging, to see what was registered and which types were created. See [Debugging](../working-with/debugging.md).

The format of the dumps is meant for humans. It may change between versions, so don't parse it.
