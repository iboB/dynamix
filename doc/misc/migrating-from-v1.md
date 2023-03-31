# Migrating from v1

## Compatibility Library

Likely the easiest begin the migration v1 to v2 is to use the compatibility library. It provides a compatibility domain instance, object, and compatibility macros for defining messages and mixins. 

A compatibility library is included in this repo: `v1compat`. In CMake it's aliased as `dynamix::v1compat`. It is not a header only library. One needs to link with it to use it. 

It domain instance, as all v1 mixins are implicitly registered in the v1 domain. The compatibility domain sorts mixins by canonical order just as v1 did.

The compatibility macros have the same names as the macros in v1, but they are prefixed with `DYNAMIX_V1_`, as opposed to just `DYNAMIX_`.

The compatibility library does not offer any solutions for the following differences between v1 and v2:

* Unicast priority is inverted! This could've been fixed by the compatibily library, but it would have hidden potentially dangerous bugs. Instead `priority` for unicasts issues a warning, and `upriority` should be used when an instance is fixed and priority is inverted.
* `objects::implements` will not work with `foo_msg` as an argument. Instead it must be called with a template argument `foo_msg_t`.
* v1 allocators are not supported.
* v1-style mutation rules are not supported
* When the declaring message overloads, either `DYNAMIX_MAKE_FUNC_TRAITS` must be used or the "original" message (i. e. at least one not declared as an overload for the same function name) must be visible.
* v1 helpers like `object_type_template`, `same_type_mutator`, `single_object_mutator` are not available (and they don't make much sense in v2)
* v1 multicast combinator calls are not immediately provided. Instead one must add the macro `DYNAMIX_V1_CREATE_COMBINATOR_CALL_<N>` for the messages that need them.
* Accessing the object in a default implementation does *not* happen through `dm_this`. Instead an argument `dm_self` is automatically provided. The type is `object&` or `const object&` for const messages.
