# Error Handling

DynaMix reports errors with exceptions. Since objects are composed at run time, many errors which C++ catches at compile time, like calling a method which a type doesn't have, can only be detected at run time in DynaMix.

The library's philosophy is to use exceptions for errors which can be recovered from. Every exception which the library throws leaves the library and the objects involved in a valid state, and in most cases unchanged.

## Exceptions

All exceptions are defined in [exception.hpp](../../code/dynamix/exception.hpp). They derive from `dynamix::exception`, which derives from `std::runtime_error`:

| Exception | Thrown by | Examples |
|---|---|---|
| `domain_error` | Registration in domains | Duplicate or empty names, a mixin or a feature which is registered in another domain, a [foreign mixin](domains.md#domains-are-separate) in a mutation |
| `type_error` | Creating types | Duplicate mixins in a type, unknown mixin names, [feature clashes](messages.md#clashes), [mutation rule](mutation-rules.md) errors, unknown type class names |
| `object_error` | Operations on objects | Missing default constructor, copy constructor, or comparison for a mixin, mutating a [sealed](mutations.md#sealed-objects) object, move assignment with different allocators |
| `feature_error` | Using features | Calling a message which the object doesn't implement, calling the [next implementation](messages.md#calling-the-next-implementation) when there's none |

The messages of the exceptions start with the name of the domain and include the type and the names of the mixins and features involved:

```
game: {} does not implement dynamix msg 'speak_msg'
game: mutate to object of type {'health'}: 'health' missing default init
game: feature clash in {'person', 'impostor'} on 'get_name_msg' between 'impostor' and 'person'
```

The messages are meant for humans. Don't parse them.

## Exception safety

| Operation | Guarantee |
|---|---|
| [Mutations](mutations.md#errors-and-exception-safety) (`mutate`, `mutate_to`, `reset_type`) | Strong: if anything throws, including a mixin constructor, the object is unchanged. |
| Creating objects (from types, with `copy`) | Strong: if anything throws, no object is created. |
| `copy_from`, `copy_matching_from`, `move_matching_from` | Basic: the object is valid, but some mixins may have been assigned. See [Copying](objects.md#copying). |
| Registration (mixins, features, rules, type classes) | If registration throws, the item is not registered. When a mixin is registered, the features which it implements are registered first. If the mixin's registration fails after that (say, because of a duplicate name), its features remain registered. They're independent of the mixin and harmless. |
| Message calls | Whatever the implementation provides. The library itself doesn't modify anything when calling messages. |

Exceptions which are thrown by user code, like mixin constructors or message implementations, propagate unchanged.

## Errors in user callbacks

Some user-provided functions report errors through return values, since they can also be called from C: the functions of `mixin_info` like `init` and `copy_init`, and the `apply` function of mutation rules. They return `dynamix::error_return_t`, an integer, which is `result_success` (zero) on success. Any other value is an error, and the library converts it to an exception which includes the value:

```
amr: applying mutation rule 'tam rule 0' to {'mesh'} failed with error -1
```

When these functions are implemented in C++, they can also throw exceptions directly.

## Errors during static initialization

Mixins, features, and type classes which are defined with the library's macros are registered during static initialization: before `main`, or when a shared library is loaded. Registration errors there, like duplicate names, are usually programming errors. Since there's nothing to catch the exception, the program is terminated.

Most debuggers and IDEs can be set to break when an exception is thrown, or when it's not caught. This shows where the error happened. When registering mixins explicitly, for example in a plugin system, the exceptions can be caught to reject the plugin without crashing the application.

## Things which are not checked

Some errors can't be detected, or checking for them would cost too much in performance-critical code. They lead to undefined behavior:

* Using an object whose type was destroyed because a mixin in it was unregistered.
* Using a reference to a type which was destroyed by `garbage_collect_types`.
* Using pointers to internal mixins after a mutation. See [Internal and external mixins](mixins.md#internal-and-external-mixins).
* Calling `object_of` or `dm_this` with a pointer which is not to a mixin in an object.
* Unchecked access, like `unchecked_get_at`.
* The scenarios in [Dangerous Functionalities](../advanced/danger.md).
