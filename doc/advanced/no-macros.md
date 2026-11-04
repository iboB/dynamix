# Mixins Without Macros

The macros of DynaMix generate code which could be written by hand. Most of them register things in global domains during static initialization. Writing that code by hand is useful when:

* Mixins need to be registered in a domain which is not global, like a local variable or a member of a class.
* Registration should happen at a specific point in time, like in an initialization function, rather than during static initialization.
* Registration errors need to be handled, like when loading [plugins](plugins.md).
* Mixins are created at run time, for example by scripts or from data.
* The same class should be a mixin in multiple domains.

> [!TIP]
> The code on this page is from [example/guide/e-no-macros.cpp](../../example/guide/e-no-macros.cpp).

## Contents

* [Mixin info data and the builder](#mixin-info-data-and-the-builder)
* [The typed API](#the-typed-api)
* [Mixins defined at run time](#mixins-defined-at-run-time)
* [Errors](#errors)
* [Lifetimes](#lifetimes)

## Mixin info data and the builder

The core of a mixin is its [`mixin_info`](../../code/dnmx/mixin_info.h). `DYNAMIX_DEFINE_MIXIN` creates a [`util::mixin_info_data`](../../code/dynamix/mixin_info_data.hpp): a `mixin_info` together with storage for the mixin's features, mutation rules, and other data. Then it fills it with a `util::mixin_info_data_builder` and registers it.

The same can be done by hand, in any domain:

```c++
domain dom("local");

// the same builder which DYNAMIX_DEFINE_MIXIN uses
util::mixin_info_data_builder<transform>(transform_data, "transform")
    .implements<describe_msg>();
transform_data.register_in(dom);
```

The builder's constructor takes the data to fill and the mixin's name, and it infers everything it can from the class, just like `DYNAMIX_DEFINE_MIXIN`. All the builder methods from [Defining Mixins](../cpp/mixins.md#builder-reference) are available. The user-defined literals for bids and priorities are in `dynamix::util::builder_literals`.

`mixin_info_data::register_in(domain)` registers the mixin and its mutation rules in the domain, and `unregister_from(domain)` unregisters them.

Messages and other features don't depend on domains, so they can be declared and defined with macros as usual.

## The typed API

Typed functions like `add<transform>()` and `obj.get<transform>()` get the mixin's info with `dynamix::g::get_mixin_info<transform>()`. It calls a function `_dynamix_get_mixin_info(const transform*)`, which is found by argument-dependent lookup. `DYNAMIX_DECLARE_MIXIN` declares it, and `DYNAMIX_DEFINE_MIXIN` defines it. Defining it by hand enables the typed API for mixins which are registered without macros:

```c++
// the declaration allows the typed API: add<transform>(), get<transform>()
DYNAMIX_DECLARE_MIXIN(transform);

// mixin info data: the mixin info, along with storage for its features and other data
dynamix::util::mixin_info_data transform_data;

// this is what DYNAMIX_DEFINE_MIXIN would define
const dynamix::mixin_info& _dynamix_get_mixin_info(const transform*) {
    return transform_data.info;
}
```

Since there's a single such function per class, the typed API can work with a single domain per class. Mixins without it can still be used through their info or by name:

```c++
// a mixin without a declaration can only be used by info or by name
util::mixin_info_data health_data;
util::mixin_info_data_builder<health>(health_data, "health")
    .implements<describe_msg>(1_bid);
health_data.register_in(dom);
```

```c++
object obj(dom);
mutate(obj, add<transform>(), add("health"));
obj.get<transform>()->x = 3;
std::cout << describe(obj) << '\n'; // health 100
```

To use the same class as a mixin in multiple domains, create a `mixin_info_data` for each domain, and use the infos (or names) to refer to the mixin.

## Mixins defined at run time

A mixin doesn't need to be a C++ class. All the library needs is a `mixin_info` with a name, a size and alignment, and functions which create, copy, move, and destroy instances. This allows defining mixins at run time, for example for objects which are defined by scripts or data:

```c++
// a mixin defined at run time: for example by a script or by data
// it's a buffer of 16 bytes, initialized with zeroes and copied with memcpy
std::string runtime_name = "blob";
common_mixin_info blob(runtime_name);
blob.set_size_alignment(16, 8);
blob.init = dnmx_mixin_common_init_func;
blob.copy_init = dnmx_mixin_common_copy_func;
blob.copy_asgn = dnmx_mixin_common_copy_func;
blob.move_init = dnmx_mixin_common_move_func;
blob.move_asgn = dnmx_mixin_common_move_func;
dom.register_mixin(blob);

mutate(obj, add(blob));
```

`common_mixin_info` is a `mixin_info` with a constructor which sets sane defaults. Its fields are documented in [mixin_info.h](../../code/dnmx/mixin_info.h). In short:

* `set_size_alignment(size, alignment)` must be used to set the size and alignment, since it also calculates the size of external buffers.
* `init`, `copy_init`, `move_init`, `copy_asgn`, `move_asgn`: a `nullptr` means that the operation is not allowed. A `nullptr` `move_init` makes the mixin external.
* `destroy`: a `nullptr` means that nothing has to be done.
* `features`: an array of `feature_for_mixin`, set with `set_features`.
* `user_data`: anything. For example a pointer to the script class which defines the mixin, which the mixin's functions can use. They all receive the mixin info as their first argument.

`util::common_init_mixin_info<T>(info)` from [mixin_info_util.hpp](../../code/dynamix/mixin_info_util.hpp) sets these functions for a C++ type. [mixin_common_funcs.h](../../code/dnmx/mixin_common_funcs.h) has implementations for plain data.

## Errors

Unlike registration during static initialization, explicit registration happens where exceptions can be caught:

```c++
// explicit registration errors can be caught
common_mixin_info other_blob(runtime_name); // duplicate name
try {
    dom.register_mixin(other_blob);
}
catch (const domain_error& e) {
    std::cout << "error: " << e.what() << '\n';
}
```

```
error: local: register mixin with duplicate name 'blob'
```

## Lifetimes

The domain doesn't copy mixin infos. They must outlive their registration. Before unregistering a mixin, make sure no objects have it. Unregistering it destroys all types which contain it.

```c++
// objects must not have mixins which are being unregistered
obj.clear();
dom.unregister_mixin(blob);
health_data.unregister_from(dom);
transform_data.unregister_from(dom);
```

Features which were registered by mixins stay registered in the domain after the mixins are unregistered. If a feature's info is going to be destroyed before the domain (for example if it's in a plugin which is unloaded), unregister it with `domain::unregister_feature`.

Destroying a domain doesn't reset the infos which are registered in it. If they need to be reused (for example registered in another domain), unregister them first.