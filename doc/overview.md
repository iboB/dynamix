# Overview

DynaMix is a new approach to OOP and dynamic polymorphism. It is a library which allows the composition of polymorphic objects at run time. The objects are composed of building blocks called *mixins*  which provide *abstract features* to the objects and allow client code to make use of them while remaining oblivious to the concrete composition.

The result is similar to [multiple inheritance in C++](https://www.learncpp.com/cpp-tutorial/multiple-inheritance/), but more closely resembles [mixins in Ruby](https://www.tutorialspoint.com/ruby/ruby_modules.htm), [Dart](https://dart.dev/language/mixins) and [Scala](https://docs.scala-lang.org/tour/mixin-class-composition.html), traits in [Self](https://handbook.selflanguage.org/2017.1/glossary.html), [PHP](https://www.php.net/manual/en/language.oop5.traits.php), and [others](https://en.wikipedia.org/wiki/Trait_(computer_programming), the [roles in Perl](https://docs.raku.org/language/objects#Roles) or [inheritance in Eiffel](https://www.eiffel.org/doc/eiffel/I2E-_Inheritance).

It must be noted that the library is a means to create a project's *architecture* rathern than implement its purpose. The library doesn't *do* anything, but introduces idioms and a paradigm by which one can create the interface for what's being done. In a way it can be viewed as a language extention rather than a utility.

It in this regard it can be compared to [COM](https://en.wikipedia.org/wiki/Component_Object_Model) which is a library that introduces more orhtodox (in the style of Java or C#) type of dynamic polymorphism to C. A [list of more comparisons](misc/dynamix-vs-x.md) of DynaMix to existing solutions is available.

## Library name

DynaMix is a portmanteau which stands for "dynamic mixins".

In C++ circles the term "mixin" has gained some popularity. In this context a mixin is a building block for a type, which interacts via other building blocks via [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). This is a way to accomplish static polymorphism and everything is resolved at compile time.

The purpose of DynaMix is to accomplish something very similar, but at run time, in a dynamic manner, and hence the name is Dynamic Mixins.

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

## When (and when not) to use DynaMix

The more complex the objects in a piece of software are, the more beneficial it will be to use the library. Pieces of software that typically have very complex objects include games (especially role-playing ones or strategies), CAD systems, enterprise systems, UI libraries, and others.

As a general rule of thumb: if you need complex polymorphic objects, DynaMix is a likely good choice.

We should emphasize on the polymorphism. In many very high-performance systems polymorphism is avoided at the cost of code that is (at least somewhat) harder to write and maintain (this is most often the case with high-end games). Since such systems will try to "squeeze" every possible piece of processing power out of the CPU, cache locality and lack of cache misses are critical in some parts of their code. As is the case with all instances of polymorphism, including C++ virtual methods and `std::function`, uses of DynaMix will almost certainly lead to cache misses. Of course, you may still rely on the library in other parts of your code, like the business (or gameplay) logic. For more information about the library performance, see [Performance](working-with/perf.md).

Of course, small projects with simple objects, even if they are polymorphic, may end up not finding any particular benefits in using the library, since their size makes them fast to compile and easy to maintain as they are. If a piece of software can be created in a couple of days, by one or two programmers, there will hardly be any need for DynaMix.
