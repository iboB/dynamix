# The C Interface

DynaMix has a C interface. It's in the [`code/dnmx`](../../code/dnmx) directory, its headers are included as `<dnmx/...>`, and it's part of the same library as the C++ interface, so nothing extra is needed to use it. It requires C11.

The core of the library is written in C++ and the C interface wraps it (see the [FAQ](../misc/faq.md#why-the-c-core-why-not-c) for why). The most performance-critical parts, like accessing mixins in objects and features in types, are implemented in C and available as inline functions, so they have no overhead in C.

The C interface follows the same concepts as the C++ one. This page describes how it differs. Read the [C++ Guide](../README.md#c-guide) for the concepts themselves.

> [!TIP]
> The code on this page is from [example/guide/c_interface.c](../../example/guide/c_interface.c). It's a complete program which is built and run with the library's tests. [example/hello-c-world](../../example/hello-c-world/hello-world.c) is a smaller one.

## Contents

* [Correspondence with the C++ interface](#correspondence-with-the-c-interface)
* [Domains](#domains)
* [Features](#features)
* [Mixins](#mixins)
* [Mutations](#mutations)
* [Errors](#errors)
* [What's missing](#whats-missing)

## Correspondence with the C++ interface

| C++ | C | Header |
|---|---|---|
| `dynamix::domain` | `dnmx_domain_handle` | `dnmx/domain.h` |
| `dynamix::object` | `dnmx_object_handle`, `dnmx_const_object_handle` | `dnmx/object.h` |
| `dynamix::type` | `dnmx_type_handle` | `dnmx/type.h` |
| `dynamix::type_mutation` | `dnmx_type_mutation_handle` | `dnmx/type_mutation.h` |
| `mutate`, `mutate_to` | `dnmx_mutate`, `dnmx_mutate_to` | `dnmx/mutate.h` |
| `dynamix::mixin_info` | `dnmx_mixin_info` | `dnmx/mixin_info.h` |
| `dynamix::feature_info` | `dnmx_feature_info` | `dnmx/feature_info.h` |
| `dynamix::mutation_rule_info` | `dnmx_mutation_rule_info` | `dnmx/mutation_rule_info.h` |
| `dynamix::type_class` | `dnmx_type_class` | `dnmx/type_class.h` |
| `std::string_view` | `dnmx_sv` | `dnmx/bits/sv.h` |

Functions follow the pattern `dnmx_<class>_<method>`. For example `obj.has(info)` is `dnmx_object_has(obj, &info)`, and `t.implements_strong(info)` is `dnmx_type_implements_strong(t, &info)`. Functions which take names have a `_by_name` suffix and take a `dnmx_sv`: `dnmx_object_has_by_name(obj, dnmx_make_sv_lit("health"))`.

The handles are pointers to the same objects which the C++ interface uses. They can be converted with `to_c_handle()` and `from_c_handle()` in C++, so C and C++ code can work with the same domains and objects.

## Domains

There are no domain tags and no global domains. Domains are created and destroyed explicitly:

```c
dnmx_domain_settings settings = {0};
dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("game"), settings, 0, NULL);
// ...
dnmx_destroy_domain(dom);
```

## Features

There are no messages in C. Features are described by `dnmx_feature_info` and their payloads are opaque pointers: `void*`. What the payload is and how it's used is up to the code which defines the feature. The most natural choice is a function which takes a pointer to the mixin, which makes features work like messages:

```c
dnmx_feature_info describe_feature; // multicast: void(const void* mixin)
dnmx_feature_info get_hp_feature; // unicast: int(const void* mixin)

// ...
describe_feature = dnmx_make_feature_info();
describe_feature.name = dnmx_make_sv_lit("describe");
describe_feature.allow_clashes = true;

get_hp_feature = dnmx_make_feature_info();
get_hp_feature.name = dnmx_make_sv_lit("get_hp");
get_hp_feature.allow_clashes = true;
get_hp_feature.default_payload = (void*)default_get_hp;
```

Calling a feature means looking up its entry in the type's feature table with `dnmx_ftable_at`, and calling the payloads. A unicast calls the first payload, and a multicast calls the payloads with the top bid in reverse order. See [How implementations are ordered](../cpp/messages.md#how-implementations-are-ordered):

```c
typedef void (*describe_func)(const void* mixin);
typedef int (*get_hp_func)(const void* mixin);

// call a unicast: the top implementation
int get_hp(dnmx_object_handle obj) {
    dnmx_ftable_entry e = dnmx_ftable_at(obj->m_type, get_hp_feature.id);
    if (!e.begin) {
        // no implementation: use the default payload (which has a different signature here)
        return ((int (*)(dnmx_object_handle))get_hp_feature.default_payload)(obj);
    }
    get_hp_func f = (get_hp_func)e.begin->payload;
    return f(dnmx_object_unchecked_get_at(obj, e.begin->mixin_index));
}

// call a multicast: all implementations with the top bid, from top_bid_back to begin
void describe(dnmx_object_handle obj) {
    dnmx_ftable_entry e = dnmx_ftable_at(obj->m_type, describe_feature.id);
    if (!e.begin) return;
    for (const dnmx_ftable_payload* p = e.top_bid_back + 1; p != e.begin; ) {
        --p;
        describe_func f = (describe_func)p->payload;
        f(dnmx_object_unchecked_get_at(obj, p->mixin_index));
    }
}
```

`dnmx_ftable_at` and `dnmx_object_unchecked_get_at` are inline, so this is as fast as messages in C++.

## Mixins

There are no mixin definition macros. A mixin is described by a `dnmx_mixin_info` which is filled in and registered explicitly:

```c
typedef struct health {
    int hp;
} health;

dnmx_feature_for_mixin health_features[2];
dnmx_mixin_info health_info;

// ...
// DNMX_COMMON_INIT_MI sets the name, the size, and functions which treat the mixin as plain data
DNMX_COMMON_INIT_MI(health_info, health);
health_features[0] = (dnmx_feature_for_mixin){.info = &describe_feature, .payload = (void*)health_describe};
health_features[1] = (dnmx_feature_for_mixin){.info = &get_hp_feature, .payload = (void*)health_get_hp};
health_info.features = health_features;
health_info.num_features = 2;
dnmx_register_mixin(dom, &health_info);
```

The info contains the size and alignment of the mixin, and functions to construct, copy, move, compare, and destroy it. Unlike C++, where they're inferred from the class, here they must be provided. The fields are documented in [mixin_info.h](../../code/dnmx/mixin_info.h). In short:

* `init`, `copy_init`, `move_init`, `copy_asgn`, `move_asgn`: if a function is `NULL`, the corresponding operation is not allowed. Notably, a `NULL` `move_init` makes the mixin [external](../cpp/mixins.md#internal-and-external-mixins).
* `destroy`: `NULL` means nothing has to be done to destroy the mixin.
* `equals`, `compare`: optional, for `dnmx_object_equals` and `dnmx_object_compare`.

[mixin_common_funcs.h](../../code/dnmx/mixin_common_funcs.h) has implementations for plain data (`memset`, `memcpy`, `memcmp`), and [mixin_info_util.h](../../code/dnmx/mixin_info_util.h) has two macros which use them:

* `DNMX_COMMON_INIT_MI(info, type)` - for plain data structs. Initializes with zeroes, copies and moves with `memcpy`.
* `DNMX_COMMON_INIT_EMPTY_MI(info, name)` - for mixins with no data (size zero), which only provide features.

The features of a mixin are an array of `dnmx_feature_for_mixin`: a feature info, the payload, and the [bid and priority](../cpp/messages.md#bids-and-priorities).

The domain doesn't copy the infos, so they must outlive their registration. Destroying a domain doesn't reset the infos which are registered in it. If you want to reuse them (say, in another domain), unregister them first.

## Mutations

`dnmx_mutate` takes an array of ops:

```c
dnmx_error_return_t init_health(dnmx_mutate_func_args args) {
    health* h = (health*)args.mixin_buf;
    h->hp = (int)args.user_data;
    return dnmx_result_success;
}

// ...
dnmx_mutate_op ops[] = {
    {.op_type = dnmx_mutate_op_add, .mixin = &health_info, .init_override = init_health, .user_data = 80},
};
if (dnmx_mutate(hero, ops, 1) != dnmx_result_success) {
    printf("mutation failed\n");
    return 1;
}
```

Each op has a type (`dnmx_mutate_op_add`, `dnmx_mutate_op_remove`, or `dnmx_mutate_op_to_back`), and a mixin, given by info or by name. Add ops can have an `init_override` function to construct the mixin instead of its default `init`, and a `user_data` value which is passed to it.

`dnmx_mutate_to` mutates an object to a type, with an array of overrides for new and common mixins. Types can be created from type mutations (`dnmx_create_type_mutation_empty`, `dnmx_get_type`) or from arrays of mixin infos (`dnmx_get_type_from_infos`).

Objects are created with `dnmx_create_object_empty`, `dnmx_create_object_default` (from a type), or `dnmx_create_object_copy`, and must be destroyed with `dnmx_destroy_object`.

## Errors

C has no exceptions. Functions which can fail return `dnmx_error_return_t`, which is `dnmx_result_success` (zero) on success, and nonzero on failure. Some return `bool` or a null handle instead. Functions which in C++ would return a value and throw on error return a struct with the value and an error, like `dnmx_int_result` of `dnmx_object_compare`.

User functions, like `init` of mixin infos, `init_override` of mutate ops, or the `apply` of mutation rules, report errors the same way. A nonzero return value aborts the operation and leaves the object unchanged, just like an exception in C++.

## What's missing

The C interface supports everything which is needed to work with DynaMix in C, but some things are less convenient than in C++, and some are missing:

* There are no messages. Features have to be called through the feature table, as shown above.
* There's no equivalent of `object_of`: a mixin can't get its object.
* There's no `add_if_lacking` mutate op. Use a type mutation with `dnmx_type_mutation_add_if_lacking` and `dnmx_mutate_to`.
* Errors are reported as error codes without messages.
* Domains and objects can't have custom allocators.
