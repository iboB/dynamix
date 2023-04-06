# Overview

DynaMix is a new approach to dynamic polymorphism. It is a library which allows the composition of polymorphic objects at run time. The objects are composed of building blocks called *mixins*  which provide *abstract features* to the objects and allow client code to make use of them while remaining oblivious to the concrete composition.

The result is similar to [multiple inheritance in C++](https://www.learncpp.com/cpp-tutorial/multiple-inheritance/), but more closely resembles [mixins in Ruby](https://www.tutorialspoint.com/ruby/ruby_modules.htm), [Dart](https://dart.dev/language/mixins) and [Scala](https://docs.scala-lang.org/tour/mixin-class-composition.html), traits in [Self](https://handbook.selflanguage.org/2017.1/glossary.html), [PHP](https://www.php.net/manual/en/language.oop5.traits.php), and [others](https://en.wikipedia.org/wiki/Trait_(computer_programming), the [roles in Perl](https://docs.raku.org/language/objects#Roles) or [inheritance in Eiffel](https://www.eiffel.org/doc/eiffel/I2E-_Inheritance). 

It must be noted that the library is a means to create a project's *architecture* rathern than implement its purpose. The library doesn't *do* anything, but introduces idioms and a paradigm by which one can create the interface for what's being done. In a way it can be viewed as a language extention rather than a utility.

It can be compared to [COM](https://en.wikipedia.org/wiki/Component_Object_Model) which is a library that introduces more orhtodox (in the style of Java or C#) type of dynamic polymorphism to C. This documentation has a [list of more comparisons](misc/dynamix-vs-x.md) of DynaMix to existing solutions.

## Key library features

## When (and when not) to use DynaMix
