# Mutation Rules

Mutation rules are functions which are applied to every mutation in a domain. They can change the resulting type: add mixins, remove them, or reorder them. They're a way to enforce invariants of the composition of objects in a single place, instead of in every piece of code which mutates objects.

Some examples of what mutation rules can do:

* A mixin needs another to work: physics needs a body.
* Adding a mixin should also add another: every character starts with an inventory.
* Mixins are mutually exclusive: an object can't be both a ghost and have physics.
* A mixin is mandatory: all objects must have a transform.
* A mixin is deprecated and should be replaced by another.

> [!TIP]
> The code on this page is from [example/guide/mutation_rules.cpp](../../example/guide/mutation_rules.cpp). It's a complete program which is built and run with the library's tests.

## Contents

* [Common rules](#common-rules)
* [Dependencies](#dependencies)
* [Custom rules](#custom-rules)
* [How rules are applied](#how-rules-are-applied)

## Common rules

The two most common rules are available in mixin definitions with `.on_mutation(rule)`:

```c++
#include <dynamix/common_mutation_rules.hpp>

DYNAMIX_DEFINE_MIXIN(game, body);
DYNAMIX_DEFINE_MIXIN(game, physics)
    .on_mutation(attaches_to<body>{})
;
DYNAMIX_DEFINE_MIXIN(game, character)
    .on_mutation(also_adds<inventory>{})
;
DYNAMIX_DEFINE_MIXIN(game, inventory);
```

The rules are registered in the domain with the mixin, and unregistered with it.

### `attaches_to<Primary>`

The defined mixin is attached to `Primary`. It's present in an object if and only if `Primary` is:

```c++
// physics attaches to body: it's added and removed together with it
object obj(dom);
mutate(obj, add<body>());
print_type(obj); // {body, physics}

mutate(obj, remove<body>());
print_type(obj); // {}

// physics can't be added without a body
mutate(obj, add<physics>());
print_type(obj); // {}
```

To achieve this, `attaches_to` makes the defined mixin a [dependency](#dependencies).

### `also_adds<Other>`

When the defined mixin is in an object, `Other` is added too, if the object doesn't already have it. Unlike `attaches_to`, `Other` is not a dependency. It's not removed when the defined mixin is removed, and it can be added and removed independently:

```c++
// character also adds inventory, but doesn't remove it
mutate(obj, add<character>());
print_type(obj); // {body, character, inventory, physics}

mutate(obj, remove<character>());
print_type(obj); // {body, inventory, physics}
```

### Rule priority

Both rules take an optional [priority](#how-rules-are-applied) and user data: `.on_mutation(attaches_to<body>{}, 4_prio)`.

## Dependencies

A mixin can be marked as a *dependency*, either explicitly with `.dependency()` in its definition, or implicitly by `attaches_to` and `adds_mutation_rule`. Dependencies exist only because of mutation rules. At the start of every mutation, all dependencies are removed from it, and it's up to the rules to add them back.

This is what makes `attaches_to` work: `physics` is removed from every mutation, and the rule adds it back only if the result has a `body`. It has some consequences:

* Adding a dependency with a mutation op has no effect unless a rule adds it.
* Dependencies are re-added by rules in every mutation, and thus in domains which don't [canonicalize types](domains.md#canonicalize_types) they move to the back of the type. Note how `physics` comes after `character` and `inventory` above.
* This doesn't mean that dependencies are destroyed and reconstructed. As long as a dependency is in the object both before and after a mutation, it's preserved.

Even though adding a dependency with an op has no effect by itself, the op is still useful. If the rules add the dependency, the op is used to construct it:

```c++
// but if it will be added by a rule, its constructor arguments can be provided
mutate(obj, add<body>(), add<physics>(5.f));
print_type(obj); // {body, physics}
std::cout << "mass: " << obj.get<physics>()->mass << '\n'; // mass: 5
```

Mixins which are added by rules without an op are default constructed.

## Custom rules

A mutation rule is described by a [`mutation_rule_info`](../../code/dnmx/mutation_rule_info.h):

```c++
#include <dynamix/mutation_rule_info.hpp>
#include <dynamix/type_mutation.hpp>

// a custom rule: ghosts have no physics
dynamix::error_return_t no_physics_for_ghosts(dnmx_type_mutation_handle hmut, uintptr_t /*user_data*/) {
    auto& mut = *dynamix::type_mutation::from_c_handle(hmut);
    if (mut.has<ghost>()) {
        mut.remove<physics>();
    }
    return dynamix::result_success;
}

const dynamix::mutation_rule_info no_physics_for_ghosts_rule = {
    dnmx_make_sv_lit("no physics for ghosts"),
    no_physics_for_ghosts,
    0, // user data
    100 // order priority: apply after the default ones (0)
};
```

| Field | Description |
|---|---|
| `name` | A name for diagnostics. |
| `apply` | The function which applies the rule. |
| `user_data` | A `uintptr_t` which is passed to `apply`. |
| `order_priority` | Rules with lower values are applied first. |

The apply function receives a handle to the [`type_mutation`](../../code/dynamix/type_mutation.hpp) of the type which is being created. A `type_mutation` has a vector of mixins, `mixins`, which can be modified directly, along with methods like `add`, `add_if_lacking`, `remove`, `to_back`, and queries like `has`, `lacks`, and `implements`.

The function returns `result_success` (zero) on success. Returning any other value is an error and the mutation throws `type_error`. The function can also throw an exception, which is propagated to the code which caused the mutation. In both cases, no type is created and the object which was being mutated is unchanged.

Rules are added to and removed from domains:

```c++
dom.add_mutation_rule(no_physics_for_ghosts_rule);

mutate(obj, add<ghost>());
print_type(obj); // {body, inventory, ghost}

mutate(obj, remove<ghost>());
print_type(obj); // {body, inventory, physics}

dom.remove_mutation_rule(no_physics_for_ghosts_rule);
```

Note how `physics` came back when `ghost` was removed. It's still attached to `body` and the custom rule no longer removes it.

The domain doesn't copy the `mutation_rule_info`, so it must outlive its registration. Rules are reference counted. Adding the same rule to a domain multiple times requires the same number of removals to remove it. Unlike mixins and features, the same rule can be added to multiple domains. Adding or removing a rule invalidates the domain's cache of [type queries](#rules-must-be-pure), but not the types themselves.

A rule can also be added by a mixin definition, with `.adds_mutation_rule(apply, user_data, perks)`. Such rules are registered and unregistered with the mixin. This is meant for rules which add the defined mixin, and so it also makes the mixin a dependency. If that's not what you need, call `.dependency(false)` after it.

## How rules are applied

When the domain is asked for a type for a mutation, it:

1. Removes the dependencies from the mutation.
2. Applies all rules in order: by `order_priority` (lower first), then by name.
3. If the rules changed the mutation, it applies them again. Since rules may depend on each other (one adds a mixin which triggers another), this is repeated until applying them doesn't change anything.

If the rules still change the mutation after several applications, they're considered cyclic and the mutation throws `type_error`. The limit is small: about five applications, or the number of rules if there are fewer. For long chains of dependent rules, use `order_priority` to apply them in the right order.

In domains which [canonicalize types](domains.md#canonicalize_types), sorting the mixins is itself a rule, with the highest order priority. It's applied last.

### Rules must be pure

The domain caches the result of every mutation: the resulting type for a given list of mixins. When the same list is mutated again, the rules are not applied: the cached type is used. For this to work, rules must be pure functions of the mutation. Their result must depend only on the mixins in the mutation, and on nothing else, like global state, time, or randomness. A rule which changes its behavior based on external state will give stale results.

If a rule's behavior has to change, remove it and add a new one. This invalidates the cache.

### Where rules are not applied

* `mutate_to(obj, type)`, `object(type)`, and `reset_type(type)` take an already created type. If it was created by a domain, the rules were applied when it was created.
* The domain's empty type, `get_empty_type()`, is not affected by rules. An empty object always has the empty type, even if there are rules which add mixins to empty mutations.
