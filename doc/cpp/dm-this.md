# Accessing the Object from a Mixin

A mixin is part of an object, much like a parent class is part of an instance of a derived class. Mixins often need to access their object: to call messages for it, or to get other mixins. With multiple inheritance this would be done with `this` or with a `dynamic_cast` of `this` to a sibling class. With DynaMix it's done with `dm_this`.

> [!TIP]
> The code on this page is from [example/guide/dm_this.cpp](../../example/guide/dm_this.cpp). It's a complete program which is built and run with the library's tests.

```c++
#include <dynamix/dm_this.hpp>

struct health {
    int hp;
    explicit health(int h) : hp(h) {}

    void take_damage(int dmg) {
        hp -= dmg;

        // access a sibling mixin through the object
        auto n = dm_this->get<name>();
        std::cout << (n ? n->value : "Something") << " has " << hp << " hp\n";

        if (hp <= 0) {
            // call a message for the object
            on_death(*dm_this);
        }
    }
};

struct loot {
    void on_death() {
        std::cout << "dropping loot\n";
    }
};
```

`dm_this` is a macro which can be used in the methods of mixins. It evaluates to a pointer to the mixin's object: `dynamix::object*`, or `const dynamix::object*` in const methods.

Here `health` doesn't know what happens when the object dies. It just sends `on_death` to its own object. Any mixin in the object which implements it reacts: in this case `loot`. This is how mixins cooperate without depending on each other.

## `object_of`

`dm_this` is a shortcut for `dynamix::object_of(this)`. The function `object_of(mixin_ptr)` from [object_of.hpp](../../code/dynamix/object_of.hpp) works with any pointer to a mixin, not only `this`. This is useful in functions which implement messages with [`implements_by`](messages.md#with-functions), where the mixin is an argument:

```c++
.implements_by<speak_msg>([](const parrot* p) {
    auto obj = dynamix::object_of(p);
    // ...
})
```

It's a constant-time operation: every mixin, internal or external, is preceded in memory by a pointer to its object. The library keeps it up to date when objects are moved.

If objects are of a class derived from `dynamix::object` (see [Objects](objects.md#deriving-from-dynamixobject)), you can `static_cast` the result to the derived class, provided that all objects which have the mixin are of that class.

> [!WARNING]
> `object_of` and `dm_this` only work with mixins which are part of objects. Calling them with a pointer to anything else (like a mixin class which was created as a standalone instance, outside of an object) is undefined behavior.

## In constructors and destructors

`dm_this` is valid in mixin constructors and destructors, but the type of the object is not reliable at that point. During a mutation, the object still has its old type while new mixins are constructed. Using messages, sibling mixins, or anything else which depends on the type is not safe. See [Dangerous Functionalities](../advanced/danger.md#accessing-the-object-from-a-mixins-constructor-or-destructor) for the details.

Mutating the object from a mixin's method is also possible, but needs care. See [Mutating an object within a message](../advanced/danger.md#mutating-an-object-within-a-message).
