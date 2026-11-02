# Objects

`dynamix::object` is the class whose instances are composed of mixins. By itself it's not much more than a container for mixins and a pointer to its [type](mutations.md#types). Everything that it can do comes from its mixins and their [features](messages.md).

> [!TIP]
> The code on this page is from [example/guide/objects.cpp](../../example/guide/objects.cpp). It's a complete program which is built and run with the library's tests.

## Contents

* [Creating objects](#creating-objects)
* [Accessing mixins](#accessing-mixins)
* [Deriving from `dynamix::object`](#deriving-from-dynamixobject)
* [Copying](#copying)
* [Moving](#moving)
* [Other operations](#other-operations)

## Creating objects

An object always belongs to a [domain](domains.md). It's created either empty, in a domain, or from a type:

```c++
dynamix::object empty(dynamix::g::get_domain<game>());

dynamix::object npc(npc_type); // default constructs the mixins of the type
```

An empty object has no mixins. Its type is the domain's empty type, which still implements features with [default implementations](messages.md#default-implementations). Objects are then composed with [mutations](mutations.md).

Both constructors take an optional `dynamix::allocator`, which is a polymorphic allocator (`std::pmr::polymorphic_allocator` or a compatible implementation where it's not available). It's used to allocate the object's mixins. See [Allocators](../advanced/allocators.md).

## Accessing mixins

```c++
// by type: returns a typed pointer or null
if (auto* t = hero.get<transform>()) t->x = 5;

// by name or by mixin info: returns void* or null
std::cout << "has health: " << hero.has("health") << '\n';
auto* h = static_cast<health*>(hero.get("health"));
h->hp = 80;

// by index in the type
std::cout << "mixins: " << hero.num_mixins() << '\n';
std::cout << "first: " << hero.get_type().mixins[0]->name.to_std() << '\n';
```

| Method | Description |
|---|---|
| `has<M>()`, `has(info)`, `has(id)`, `has(name)` | Check whether the object has a mixin. |
| `get<M>()` | Get a pointer to a mixin, or `nullptr` if the object doesn't have it. Const objects return const pointers. |
| `get(info)`, `get(id)`, `get(name)` | The same, but returns `void*`. |
| `get_at(index)` | Get the mixin at an index in the type, or `nullptr` if the index is out of range. |
| `unchecked_get_at(index)` | Get the mixin at an index without checking it. |
| `num_mixins()` | Number of mixins. |
| `empty()` | True if the object has no mixins. |
| `get_type()` | The object's [type](mutations.md#types), which has many more queries. |
| `get_domain()` | The object's domain. |
| `is_of<TypeClass>()`, `is_of(type_class)`, `is_of(name)` | Check whether the object's type matches a [type class](type-classes.md). |

Lookups by type, id, and info are constant time. Lookups by name are not, as they involve string comparisons. Prefer the others in performance-critical code.

Remember that [mutations](mutations.md#what-happens-in-a-mutation) invalidate pointers to internal mixins. Don't keep pointers to mixins across mutations.

Accessing mixins directly is useful, but the object-oriented way to work with DynaMix objects is through their features. Code which uses messages doesn't need to know which mixins an object has. Code which uses `get` does. Prefer messages for code which deals with behavior and use `get` when you need the data of a specific mixin.

## Deriving from `dynamix::object`

`dynamix::object` can be used as a base class. This is a common way to tie objects to a domain and to give them more convenient semantics:

```c++
// a class derived from dynamix::object
// it provides a domain-specific constructor, copy semantics, and stronger typing for messages
class entity : public dynamix::object {
public:
    entity() : dynamix::object(dynamix::g::get_domain<game>()) {}

    // copying
    entity(const entity& other) : dynamix::object(other.copy()) {}
    entity& operator=(const entity& other) {
        copy_from(other);
        return *this;
    }

    // the copy operations above hide the defaulted move operations, so we bring them back
    entity(entity&&) noexcept = default;
    entity& operator=(entity&&) = default;
};

// messages can take the derived class instead of dynamix::object
DYNAMIX_DECLARE_MSG(describe_msg, describe, std::string, (const entity&));
```

Messages whose first argument is a derived class can only be called for objects of that class. This is a way to make sure messages aren't called for objects of the wrong domain at compile time.

Note that `dynamix::object` has no virtual destructor and no virtual methods. Don't `delete` derived objects through pointers to `dynamix::object`.

## Copying

`dynamix::object` can't be copied implicitly: its copy constructor and copy assignment are deleted. Copying objects with many mixins can be expensive and it's easy to do by accident. Instead, copies are made explicitly:

* `copy(alloc = {})` returns a new object with the same type, whose mixins are copy constructed from the source's. The copy is not sealed, even if the source is.
* `copy_from(other)` makes the object a copy of `other`. The object's type changes to the type of `other`. Mixins which both objects have are copy assigned, new mixins are copy constructed, and mixins which `other` doesn't have are destroyed.
* `copy_matching_from(other)` copy-assigns only the mixins which both objects have. The object's type doesn't change.

If you want implicit copies, derive from `dynamix::object` and add copy operations, like `entity` above:

```c++
entity clone = hero; // copy constructor: clone has the same type and copies of the mixins

entity other;
mutate(other, add<health>());
other = hero; // copy_from: other changes its type to the one of hero

// copy only the mixins which both objects have
entity patient;
mutate(patient, add<health>());
patient.copy_matching_from(hero);
```

Copying requires the mixins to be copy constructible or copy assignable as needed. If they aren't, the copy throws `object_error`. See [Defining Mixins](mixins.md#what-dynamix-infers-from-the-class).

`copy` has a strong exception guarantee: if anything throws, no object is created. `copy_from` and `copy_matching_from` only have a basic guarantee: if a copy assignment throws, the object is valid but some of its mixins may have been assigned. The type doesn't change. For a strong guarantee, copy and then move:

```c++
auto tmp = source.copy(); // if this throws, target is not touched
target = std::move(tmp);
```

## Moving

Objects can be moved. Moving an object doesn't move its mixins. The new object takes the mixins of the source, and the source is left empty, with the domain's empty type:

```c++
// moving an object doesn't move its mixins
auto* ht = hero.get<transform>();
entity moved = std::move(hero);
std::cout << "same transform: " << (moved.get<transform>() == ht) << '\n'; // true
std::cout << "moved-from is empty: " << hero.empty() << '\n'; // true
```

So moving objects is cheap, and pointers to mixins remain valid. A moved-from object is fully usable.

The move constructor is `noexcept`. Move assignment works when both objects have the same allocator. Otherwise it throws `object_error`. Moves also transfer the [sealed](mutations.md#sealed-objects) status.

`move_matching_from(other, fallback_to_init = false)` move-assigns the mixins which both objects have. If `fallback_to_init` is true, mixins which are not move assignable are destroyed and move constructed instead.

## Other operations

| Method | Description |
|---|---|
| `clear()` | Remove all mixins. The object becomes empty. |
| `reset_type(type)` | Change the object's type. New mixins are default constructed and mixins in both types are not touched. Equivalent to `mutate_to(obj, type)`. |
| `seal()`, `sealed()` | [Seal](mutations.md#sealed-objects) the object or check if it's sealed. |
| `equals(other)`, `compare(other)` | Compare objects. See [Equality and comparison](mixins.md#equality-and-comparison). |
| `get_allocator()` | The object's allocator. |

The object of a mixin can be accessed from within the mixin. See [Accessing the Object from a Mixin](dm-this.md).
