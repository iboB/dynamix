# Custom Features

[Messages](../cpp/messages.md) are the feature type which DynaMix provides, but they're built on a general mechanism, which can be used to create other kinds of features. A feature is anything which mixins can provide to the types they're part of: a function, a value, a description, a factory, a table of functions, and so on. The library takes care of collecting the features of the mixins of a type into its feature table (the *ftable*), with all the [overriding](../cpp/messages.md#unicasts-and-overriding) rules of bids, priorities, and mixin order. What a feature *is* and how it's used is up to the code which defines it.

> [!TIP]
> The code on this page is from [example/guide/e-custom-features.cpp](../../example/guide/e-custom-features.cpp).

## Contents

* [Defining a feature](#defining-a-feature)
* [Feature traits](#feature-traits)
* [Implementing the feature in mixins](#implementing-the-feature-in-mixins)
* [Accessing the feature](#accessing-the-feature)

## Defining a feature

The example defines a feature which is a value: a max speed for types of objects. Mixins provide the value and the type with the top bid decides it. Since features belong to types, it can be queried without an object.

A feature type is a struct with three members:

```c++
struct max_speed_traits;
struct max_speed {
    using traits = max_speed_traits;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};
```

* `traits` - the [feature traits](#feature-traits), which produce payloads.
* `get_info_safe()` - returns the feature's info. It's safe to call during static initialization, so it's used when mixins are defined.
* `info` - a reference to the info, for fast access.

The definition:

```c++
const dynamix::common_feature_info& max_speed::get_info_safe() {
    static float default_max_speed = 0;
    static dynamix::common_feature_info the_info(
        dnmx_make_sv_lit("max_speed"), // name
        false, // don't allow clashes
        &default_max_speed // default payload
    );
    return the_info;
}
const dynamix::feature_info& max_speed::info = max_speed::get_info_safe();
```

The [`feature_info`](../../code/dnmx/feature_info.h) has:

* `name` - must be unique in the domain (by default).
* `allow_clashes` - whether two mixins in a type can provide the feature with the same bid and priority. See [Clashes](../cpp/messages.md#clashes).
* `default_payload` - an optional payload for types which have no mixins which provide the feature. Its meaning is up to the code which uses the feature. For messages it's the default implementation.

The info doesn't need to be registered explicitly. It's registered in a domain when the first mixin which implements it is.

`DYNAMIX_DECLARE_MSG` and `DYNAMIX_DEFINE_MSG` generate exactly this for messages, with traits which produce function pointers as payloads. A library or a project which defines its own kind of features would typically have its own macros for them.

## Feature traits

A payload for a mixin is a `dynamix::any`: a type-erased shared pointer. The traits have up to three functions which produce payloads, which correspond to the three ways in which mixins implement features:

| Builder method | Traits function | Meaning |
|---|---|---|
| `implements<Feature>()` | `make_payload_for(alloc, Mixin*)` | Produce a payload for the mixin type. The mixin pointer is always null. Its type is what matters. |
| `implements_with<Feature>(value)` | `make_payload_with(alloc, Mixin*, value)` | Produce a payload from a value, which is owned by the mixin's definition. |
| `implements_by<Feature>(ref)` | `make_payload_by(alloc, Mixin*, ref)` | Produce a payload which refers to an external object. Its lifetime is the caller's responsibility. |

The builder passes the arguments to the traits as they are. The traits only need to provide the functions which make sense for the feature, and can decide which arguments to accept. For example a `make_payload_by` which takes a non-const lvalue reference rejects temporaries, which would otherwise lead to dangling payloads.

The max speed feature only supports values:

```c++
// feature traits: they produce the payloads of features for mixins
struct max_speed_traits {
    // used by implements_with<max_speed>(value)
    // the payload is a copy of the value, owned by the mixin's definition
    static dynamix::any make_payload_with(const dynamix::allocator& alloc, void* /*mixin*/, float value) {
        return dynamix::make_any(alloc, value);
    }
};
```

`dynamix::make_any(alloc, value)` creates an owned payload. `dynamix::fwd_any(ref)` creates a payload which refers to an external object without owning it.

For comparison, the traits of messages produce function pointers: `make_payload_for` generates a function which calls a method of the mixin (this is what `DYNAMIX_MAKE_FUNC_TRAITS` creates), and `make_payload_by` takes a function. See [msg_traits.hpp](../../code/dynamix/msg/msg_traits.hpp) and [func_traits.hpp](../../code/dynamix/msg/func_traits.hpp).

## Implementing the feature in mixins

Mixins implement custom features in their definitions, just like messages, including bids and priorities:

```c++
DYNAMIX_DEFINE_MIXIN(game, walker).implements_with<max_speed>(5.f);
DYNAMIX_DEFINE_MIXIN(game, car).implements_with<max_speed>(50.f);
DYNAMIX_DEFINE_MIXIN(game, turbo).implements_with<max_speed>(80.f, 1_bid);
```

## Accessing the feature

A type's ftable has an entry for each feature, which is accessed by the feature's id: `type::ftable_at(id)`. The entry is a range of `ftable_payload`, sorted as described in [How implementations are ordered](../cpp/messages.md#how-implementations-are-ordered). Each element has the payload and the index of the mixin which provided it. The entry is empty if the type has no mixins which provide the feature.

```c++
// accessing the feature: no object needed, it works on types
float get_max_speed(const dynamix::type& t) {
    auto entry = t.ftable_at(max_speed::info.id);
    if (entry) {
        // the top implementation
        return *static_cast<const float*>(entry.begin->payload);
    }
    // the default payload
    return *static_cast<const float*>(max_speed::info.default_payload);
}

float get_max_speed(const dynamix::object& obj) {
    return get_max_speed(obj.get_type());
}
```

```c++
object obj(dom);
std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 0: default

mutate(obj, add<walker>());
std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 5

mutate(obj, remove<walker>(), add<car>());
std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 50

mutate(obj, add<turbo>());
std::cout << "max speed: " << get_max_speed(obj) << '\n'; // 80
```

The entry has:

* `begin` - the top implementation: the one a unicast would use.
* `top_bid_back` - the last implementation with the top bid. `[begin, top_bid_back]` is what a multicast would use.
* `end` - the end of the range. Implementations after `top_bid_back` are overridden by higher bids.

To get the mixin which provided a payload, use its `mixin_index` with `object::get_at` or `unchecked_get_at`. This is how messages pass the mixin to their implementations.

Lookups in the ftable are constant time. The `id` of a feature is assigned by the domain when the feature is registered. Features which are not registered have an invalid id, and `ftable_at` returns an empty entry for it.
