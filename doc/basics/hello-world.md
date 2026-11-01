# Hello World

This page walks through [example/hello-world](../../example/hello-world/hello-world.cpp), a minimal DynaMix program. It shows the four elements every DynaMix program has: a *domain*, *mixins*, a *message* (the most common kind of mixin feature), and *objects*. They're all introduced briefly in [Core Concepts](glossary.md).

The program creates an object, gives it the ability to speak, then mutates the object to change *how* it speaks while keeping the interface the same.

```
Hello from DynaMix!
Good bye from DynaMix.
```

## Building it

The example is built as part of the library when `DYNAMIX_BUILD_EXAMPLES` is on (which is the default when DynaMix is the root CMake project). In your own project, after [adding DynaMix](adding.md), you only need to link with `dynamix::dynamix`:

```cmake
add_executable(hello-world hello-world.cpp)
target_link_libraries(hello-world dynamix::dynamix)
```

## Declarations

> [!NOTE]
> In the example file all `#include` directives are at the top. Here they're shown next to the code which needs them.

DynaMix separates *declaring* things from *defining* them. Declarations are lightweight and are what other code needs in order to use mixins, messages, and domains. In a real project they'd typically be in headers.

```c++
#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>

DYNAMIX_DECLARE_MIXIN(struct hello);
DYNAMIX_DECLARE_MIXIN(struct bye);

DYNAMIX_DECLARE_MSG(speak_msg, speak, void, (const dynamix::object&));

DYNAMIX_DECLARE_DOMAIN(example);
```

* `DYNAMIX_DECLARE_MIXIN` declares that `hello` and `bye` are mixins. Note that it doesn't need the definitions of the structs. Code which uses mixins with DynaMix often doesn't need to include their definitions at all.
* `DYNAMIX_DECLARE_MSG` declares a message. The arguments are:
    * `speak_msg` - the name of the message. This is a type which identifies the feature.
    * `speak` - the name of a function which will be used to call the message. It also determines which method of the mixins will be called (more on this below).
    * `void` - the return type.
    * `(const dynamix::object&)` - the arguments. The first argument is always the object. Being `const` here means that the message can be called for `const` objects and its implementations must be `const` methods.
* `DYNAMIX_DECLARE_DOMAIN` declares a domain tag: `example`. The domain is the registry where mixins and messages live. Most projects have only one.

The message declaration also declares the function `void speak(const dynamix::object&)`. This is the interface that client code uses. It's an ordinary free function: you can call it, take its address, and forward-declare it like any other function.

## Using objects

```c++
#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

int main() {
    dynamix::object greeter(dynamix::g::get_domain<example>());
```

A `dynamix::object` is always created in a domain. `dynamix::g::get_domain<example>()` gets the global domain instance for the `example` tag. The object starts out empty: it has no mixins and therefore implements no messages.

```c++
    // Composition
    mutate(greeter, dynamix::add<hello>());
```

`mutate` changes the composition of an object. Here we add an instance of `hello` to the object. The library constructs it (with its default constructor in this case) inside the object. The object now has a new type: `{hello}`.

```c++
    // Message call
    speak(greeter);
```

This is a polymorphic call. `speak` doesn't know anything about `hello`. It looks up the `speak_msg` implementation in the object's type and calls it. Since `hello` implements it, `hello::speak` is called and we see `Hello from DynaMix!`.

```c++
    // Mutation: keep the interface, but change the implementation
    mutate(greeter,
        dynamix::remove<hello>(),
        dynamix::add<bye>()
    );

    // Message call (in mutated object)
    speak(greeter);

    return 0;
}
```

This is where DynaMix differs from what is possible with plain C++. The *same object instance* changes its type at run time. The `hello` instance is destroyed and a `bye` instance is created in its place. The call to `speak` is exactly the same as before, but now `bye::speak` is executed and we see `Good bye from DynaMix.`

The object's interface (what it can do) stayed the same, while its implementation (how it does it) changed. With C++ virtual functions, this would require destroying the object and creating a new one of a different class, invalidating all references to it.

## Definitions

Everything from here on is what client code doesn't need to see. In the example it's in the same file for brevity, but it could just as well be in other compilation units, or even in [shared libraries and plugins](../../test/app-plugin/).

```c++
struct hello {
    void speak() const {
        std::cout << "Hello from DynaMix!" << std::endl;
    }
};

struct bye {
    void speak() const {
        std::cout << "Good bye from DynaMix." << std::endl;
    }
};
```

The mixins are plain classes. They have no common parent, no virtual functions, and no DynaMix-specific code. Each has a `speak` method that matches the message signature (minus the object argument).

```c++
#include <dynamix/define_mixin.hpp>

DYNAMIX_DEFINE_MIXIN(example, hello).implements<speak_msg>();
DYNAMIX_DEFINE_MIXIN(example, bye).implements<speak_msg>();
```

`DYNAMIX_DEFINE_MIXIN` registers the class as a mixin in the `example` domain. It returns a builder on which you list the features that the mixin implements and other properties of the mixin. Here, both mixins implement `speak_msg`.

```c++
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

DYNAMIX_MAKE_FUNC_TRAITS(speak);
DYNAMIX_DEFINE_MSG(speak_msg, unicast, speak, void, (const dynamix::object&));
```

* `DYNAMIX_MAKE_FUNC_TRAITS(speak)` generates the glue which makes `.implements<speak_msg>()` bind the message to a method called `speak` in the mixin. It's needed once per function name, regardless of how many messages share that name (for example as overloads).
* `DYNAMIX_DEFINE_MSG` defines the message and the `speak` function. The arguments repeat the declaration, with the addition of `unicast`: when the message is called, a single implementation is executed. The alternative is `multicast`, where all implementations in an object are executed. See [Messages](../cpp/messages.md) for more.

```c++
#include <dynamix/define_domain.hpp>

DYNAMIX_DEFINE_DOMAIN(example);
```

Finally, the domain is defined. This creates the domain instance which `dynamix::g::get_domain<example>()` returns.

## Why the macros?

Most of the macros generate global registration code, so that mixins and messages are registered in their domain before `main`, wherever they happen to be defined. This is what allows the definitions to be spread across compilation units and plugins without an explicit initialization step. The macros are not mandatory, though. Everything they do can be written by hand. See the [FAQ](../misc/faq.md#whats-with-all-these-macros).

## Next steps

* [Core Concepts and Glossary](glossary.md) - the elements of DynaMix in more detail
* [Messages](../cpp/messages.md) - everything about messages: unicasts, multicasts, overriding, default implementations, and more
