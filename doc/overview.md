# Overview

DynaMix is a new approach to OOP and dynamic polymorphism. It's a library that lets you compose polymorphic objects at runtime. The objects are built from components called *mixins* which provide *abstract features* to the objects, allowing client code to use them without knowing the concrete composition.

The result is similar to [multiple inheritance in C++](https://www.learncpp.com/cpp-tutorial/multiple-inheritance/), but more closely resembles [mixins in Ruby](https://www.tutorialspoint.com/ruby/ruby_modules.htm), [Dart](https://dart.dev/language/mixins), and [Scala](https://docs.scala-lang.org/tour/mixin-class-composition.html), traits in [Self](https://handbook.selflanguage.org/2017.1/glossary.html), [PHP](https://www.php.net/manual/en/language.oop5.traits.php), and [others](https://en.wikipedia.org/wiki/Trait_(computer_programming)), the [roles in Perl](https://docs.raku.org/language/objects#Roles), or [inheritance in Eiffel](https://www.eiffel.org/doc/eiffel/I2E-_Inheritance).

It's important to note that the library is a means to create a project's *architecture* rather than implement its purpose. The library doesn't *do* anything, but introduces idioms and a paradigm for creating the interface for what's being done. In a way, it can be viewed as a language extension rather than a utility.

In this regard, it can be compared to [COM](https://en.wikipedia.org/wiki/Component_Object_Model), which is a library that introduces a more orthodox (in the style of Java or C#) type of dynamic polymorphism to C. A [list of more comparisons](misc/dynamix-vs-x.md) of DynaMix to existing solutions is available.

## Key library features

* Compose objects from mixins at run time
* Physically separate interface and implementation
* Non-intrusive &ndash; mixins don't need to have a common parent or any special code inside
* Mutate "live" objects by changing their composition at run time
* Use `std::polymorphic_allocator` to allow fine-tuning allocations and achieving cache locality in critical parts of the code
* Create shared libraries and plugins which can enrich or modify objects without modifying (or even rebuilding) the executable.
* Add "hotswap" to a project while developing
* Have complete runtime reflection by symbols or strings
* Messages:
    * Fast polymorphic calls &ndash; comparable to `std::function`
    * Allow multicast messages &ndash; ones that are handled by many mixins within an object
    * Thread safe message calls &ndash; as thread safe as the underlying methods.

In classic OOP terminology, DynaMix is a solution with the following features:

* Dynamic type composition
* Runtime polymorphism
* Singular dispatch
* Open methods
* Late binding
* Multicast
* Reachable overrides

## DynaMix vs Multiple Inheritance in C++

The closest thing to DynaMix available in C++ is multiple inheritance. We use comparisons to multiple inheritance throughout this documentation to illustrate the library's features.

You could even say that DynaMix provides the *exact* same features as multiple inheritance. The difference is that DynaMix does this at runtime, while multiple inheritance is done at compile time. 

When using DynaMix, you can think of the mixins that compose a DynaMix type as the parents of a C++ type in a multiple inheritance hierarchy. Just like a C++ type inherited from multiple parents obtains the members of all its parents, a DynaMix type composed of multiple mixins obtains the *features* of all of its mixins. An object instance of a C++ type composed of multiple parents implicitly instantiates all of its parents, while an object instance of a DynaMix type composed of multiple mixins implicitly instantiates all of its mixins.

In a multiple inheritance hierarchy, it is popular to have a common parent class which is inherited virtually from all building blocks. This can be compared to `dynamix::object`.

To access a sibling in a multiple inheritance hierarchy, you would use `dynamic_cast` to cast `this` to the type of the sibling. To access a mixin in a DynaMix type, you would use `dynamix::object::get` to get a reference to the mixin.

Here are the most important differences between polymorphism with multiple inheritance and DynaMix:

* **Late binding**: In C++ you need to know the type of an object at compile time to polymorphically call a member function. With DynaMix, you can polymorphically call a feature of a type without having to know the type at compile time. This is not possible with C++ types.
* **No combinatorial explosion of types**: You compose types at runtime, and don't need to explicitly list all possible combinations of building-block types.
* **No type-bound interfaces**: Since the interface is physically separated from the implementation (or type), you don't need to have a single class implement an interface. You could separate the implementation of an interface between multiple mixins. To have this with C++ polymorphism, you would need to define a separate interface class for each and every virtual function you want to implement. 
* **Live object mutation**: Since the composition of a type is done at runtime, you can change the composition of an object instance at any time. This is not possible with multiple inheritance.
* **Type-level polymorphism**: In C++ you need an instance of a type to polymorphically call a member function. With DynaMix, you can polymorphically access a feature of a type without having to instantiate an object of that type. This is not possible with C++ types.
* **Runtime reflection**: DynaMix offers more reflection and introspection capabilities than what is possible with `typeid`.
* **Open interface**: If a C++ library offers a polymorphic interface with virtual functions, the way to extend the library is to create new implementations of its interfaces. With DynaMix, you can naturally do that, but you can also create new features that extend the interface as opposed to the implementation. This is not possible with C++ types.

## Library name

DynaMix is a portmanteau which stands for "dynamic mixins".

In C++ circles the term "mixin" has gained some popularity. In this context a mixin is a building block for a type, which interacts with other building blocks via [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). This is a way to accomplish static polymorphism where everything is resolved at compile time (no virtual functions are used, no common parent, `static_cast` is used to cast between types).

CRTP mixins take multiple inheritance to the static end of the polymorphism spectrum. DynaMix uses mixins to achieve dynamic polymorphism, more so than what is possible with plain multiple inheritance, and hence the name is Dynamic Mixins.

## When (and when not) to use DynaMix

The more complex the objects in a piece of software are, the more beneficial it will be to use the library. Software that typically has very complex objects includes games (especially role-playing ones or strategies), CAD systems, enterprise systems, UI libraries, and others.

As a general rule of thumb: if you need complex polymorphic objects, DynaMix is likely a good choice.

We should emphasize the polymorphism aspect. In many very high-performance systems polymorphism is avoided at the cost of code that is (at least somewhat) harder to write and maintain (this is most often the case with high-end games). Since such systems will try to "squeeze" every possible piece of processing power out of the CPU, cache locality and lack of cache misses are critical in some parts of their code. As is the case with all instances of polymorphism, including C++ virtual methods and `std::function`, uses of DynaMix will almost certainly lead to cache misses. Of course, you may still rely on the library in other parts of your code, like the business (or gameplay) logic. For more information about the library performance, see [Performance](working-with/perf.md).

Of course, small projects with simple objects, even if they are polymorphic, may not find any particular benefits in using the library, since their size makes them fast to compile and easy to maintain as they are. If a piece of software can be created in a couple of days by one or two programmers, there will hardly be any need for DynaMix.
