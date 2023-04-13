# Glossary

## Object

An object in terms of DynaMix is an instance of the class `dynamix::object`. By itself it's not much more than an empty class. Its main purpose is to be a "container" of mixin instances. You can construct an empty object and then add or remove mixins from it via *mutations*.

The particular set of mixins in an object defines its *type*. An object mutation (adding or removing mixins) changes the objects type.

"Type", as mentioned above, has nothing to do with the concept of type in C++. A `dynamix::object` naturally always is a `dynamix:object`. "Type" in this case is a runtime concept, which contains information for the composition of an object and its interface or list of *abstract features*.

An object *has* mixins and *implements* features.

## Object Type

The type is basically a list of mixin infos, which is used to create objects. It also has indexes to those infos and a materialized index of *features* called a `ftable`. Thus looking up concrete mixins and features in a type (or object) is always O(1).

The object type is ordered. A type composed of mixins `{a, b, c}` is different from a type composed of `{b, a, c}`.

The type provides utitliy functionalities to make these lookups and other queries.

Like the object, the type *has* mixins and *implements* features.

## Mixin

A mixin is the building block of types and objects. It not a specific type, but a definition. It the jobs of the library's users to define their own mixins.

Once you have mixins, you can combine them into objects. Adding or removing mixins will internally allocate and instantiate them, and destroy them. This means that a mixin instance is bound to an object instance. Objects cannot share mixin instances and only a single instance of a specific mixin can be part of an object.

You can loosely think of mixins as the parents of a class in a multiple inheritance scenario.

Mixins are defined through [`mixin_info`](../../code/dnmx/mixin_info.h). Utilities exist for common ways to define it. In the vast majority of cases a mixin will be defined as an existing type (`class` or `struct`).

The mixin *provides* feature implementations.

## Mixin Feature

The mixin features are a list of abstract features which are provided by a mixin to a type. Using the multiple inheritance example from above, much like a class inherits methods, static members, and typedefs from multiple parents, types and objects obtain the *abstract mixins features* of the mixins they are composed of.

A feature is a definition. A *feature implementation* is a provided by a mixin. An implementation on its own is nothing but a completely type erased (`void`) pointer.

DynaMix provides some feature types, and others can be created by the users. Defining a feature type would involve creating utility functionalities which reify the feature to what it needed.

### Bid and Priority

Features have two signed integer properties, called bid and priority which are used to sort them in the `ftable`. If a feature is provided by a single mixin in a type, bid and priority are ignored, but if many mixins provide the same feature, they are used to disambiguate.

Bid is the major sort key. The higher the bid, the earlier the feature will be added in the `ftable`. Features with a higher bid (say 34) *overrides* features with a lower bid (say -5).

Priority is used as an additional sort key for features with the same bid. It is however treated with the oposite value. The smaller the priority, the earlier the feature will be added in the ftable. With a same bid, a feature with a smaller priority (say -1) *precedes* features with a bigger priority (say 10).

When bid *and* priority are the same, the order in which the implementations appear in the `ftable` is the opposite of the mixin order in the type. The last mixin in the type will add the first features in the `ftable`.

### Messages

Messages are a feature type provided by the library for the C++ interface. The term "message" here is used in the same way as it is in Smalltalk or Objective C. The message is what is called, while the method is what is executed.

The message implementation is a function.

A message is a feature which is associated with function. Calling a message for an object, will lead to a function being executed with the mixin which provided it as a first argument or, if the mixin is associted with a C++ type, it can lead to a method of that type being executed.

#### Multicast

Multicast messages make use of bids and priority to execute all top bidders in the ftable for a single message call.

## Mutation

Mutations are what is used to compose types and change object types.

This is basically a way to add, remove, or reorder the mixins in a type.

Type mutations work on types, while object mutations can work on an object and its type at the same time.

### Mutation Rule

Mutation rules are functions which are applied to all mutations in a domain. The most common case would be to associate a mixin with another, but they can do anything for example prevent mixins from being added, or replacing mixins with others.

The library offers some mutation rules, but users can define their own.

## Type Class

A type class is a boolean function for a type. It can be used to categorize objects based on their mixin or feature composition.

## Domain

The domain is a library instance of sorts. The domain contains registries for mixins, features, and types (but not of objects!) and should be used to contain a specific, well... domain of use of DynaMix.

It is not possible to create types or objects using mixins from different domains.
