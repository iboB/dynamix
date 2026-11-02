# Type Classes

A type class is a named predicate for [types](mutations.md#types): a function which checks whether a type belongs to a category. It can be based on anything which is known about the type: which mixins it has, which features it implements, or a combination of them.

Type classes allow code to categorize objects by what they are or what they can do, without checking for specific mixins or features in many places. If the definition of the category changes, only the type class needs to be updated.

> [!TIP]
> The code on this page is from [example/guide/type_classes.cpp](../../example/guide/type_classes.cpp). It's a complete program which is built and run with the library's tests.

## Declaring and defining type classes

```c++
#include <dynamix/declare_type_class.hpp>

DYNAMIX_DECLARE_TYPE_CLASS(renderable);
DYNAMIX_DECLARE_TYPE_CLASS(physical);
```

```c++
#include <dynamix/define_type_class.hpp>

// a type class is defined by a function which checks a type
bool is_renderable(dnmx_type_handle ht) noexcept {
    return dynamix::type::from_c_handle(ht)->implements_strong<render_msg>();
}
DYNAMIX_DEFINE_TYPE_CLASS_WITH(game, renderable, is_renderable);

bool is_physical(dnmx_type_handle ht) noexcept {
    auto t = dynamix::type::from_c_handle(ht);
    return t->has<transform>() && t->has<body>();
}
DYNAMIX_DEFINE_TYPE_CLASS_WITH(game, physical, is_physical);
```

`DYNAMIX_DECLARE_TYPE_CLASS(name)` declares a type class. `DYNAMIX_DEFINE_TYPE_CLASS_WITH(domain_tag, name, func)` defines it with a match function, and registers it in the domain, so that it can also be referred to by name.

The match function receives a C handle to the type, which can be converted to `const dynamix::type*` with `type::from_c_handle`. It must be `noexcept`.

When the type class is in a shared library and used outside of it, use `DYNAMIX_DECLARE_EXPORTED_TYPE_CLASS(export_macro, name)`.

## Using type classes

```c++
for (auto& obj : objects) {
    if (obj.is_of<renderable>()) render(obj);
}

// type classes work on types, too
for (auto& obj : objects) {
    auto& t = obj.get_type();
    std::cout << "physical: " << t.is_of<physical>() << '\n';
}

// and they can be referred to by name
std::cout << "renderable: " << objects[2].is_of("renderable") << '\n';
```

Objects and types have:

* `is_of<TypeClass>()` - check by the declared type class.
* `is_of(const type_class&)` - check by a `type_class` instance.
* `is_of(name)` - check by the name of a type class which is registered in the domain. Throws `type_error` if there's no such type class.

The result is not cached. Every call to `is_of` calls the match function. Keep match functions simple, or cache their results for performance-critical code.

## Type classes without macros

A type class is a plain struct, [`dynamix::type_class`](../../code/dnmx/type_class.h), with a name and a match function. It can be used without registration:

```c++
dynamix::type_class has_mesh = {dnmx_make_sv_lit("has_mesh"), [](dnmx_type_handle ht) noexcept {
    return dynamix::type::from_c_handle(ht)->has<mesh>();
}};

if (obj.is_of(has_mesh)) { /*...*/ }
```

To make it available by name, register it with `domain::register_type_class` and unregister it with `domain::unregister_type_class`. The domain doesn't copy it, so it must outlive its registration. Type class names must be unique within a domain.
