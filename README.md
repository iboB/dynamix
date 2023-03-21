# [![DynaMix](https://s22.postimg.cc/u1sqhb7jl/README.jpg)](https://ibob.github.io/dynamix/)

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT) [![Build](https://github.com/iboB/dynamix/actions/workflows/unit-test.yml/badge.svg)](https://github.com/iboB/dynamix/actions/workflows/unit-test.yml)

> *IMPORTANT* v2 is here. It is a big change. The last v1 release was [v1.4.0](https://github.com/iboB/dynamix/releases/tag/v1.4.0)

DynaMix (*Dynamic Mixins*) is an alternative take on object oriented programming and dynamic polymorphism. It lets users compose and modify polymorphic objects at run time. The main target language is C++, but C is also supported.

The library is a means to create a project's **architecture** rather than achieve its purpose. It helps with *extensibility*, *readability*, *scalability* and *interoperability*. It focuses on **maximal performance** and **minimal memory overhead**.

DynaMix is applicable for the software architecture of systems with complex objects including, but not limited to:

* Games (especially role-playing ones or strategies)
* CAD systems
* Enterprise systems

The library uses the type `dynamix::object` as a placeholder, whose instances can be extended with existing classes (mixins), thus providing a particular instance with the *mixin features* of all those types. Likely the most important types of *mixin features* are messages: functional objects which in C++-OOP terms can be thought of as methods. Mixin features are overridable and use late binding and singular dispatch. Unicasts and multicasts are possible.

Here is a small example of what code may look like using the library:

```c++
    // assuming my_objects.get_ally(0); is a way to get an ally to the
    // main character in a game
    dynamix::object& obj = my_objects.get_ally(0);

    // now let's make the object think some positive thoughts about the
    // main character

    think(obj); // C++ doesn't allow us to have obj.think().
                // DynaMix's messages are like standalone functions

    // composition
    mutate(obj, dynamix::add<flying_creature>());

    // object can now respond to fly()

    fly(obj); // ...instead of obj.fly()

    // mutation
    mutate(obj
        , dynamix::remove<ally>()
        , dynamix::add<enemy>()
    );

    think(obj); // the same object now thinks negative thoughts about the main
                // character, since it's no longer an ally, but an enemy
```

Here are some of the key features of the library:

* Compose objects from mixins at run time
* Physically separate interface and implementation
* Non-intrusive &ndash; mixins don't need to have a common parent or any special code inside
* Mutate "live" objects by changing their composition at run time
* Use `std::polymorphic_allocator` to allow fine-tuning allocations and achieving cache locality in critical parts of the code
* Create shared libraries and plugins which can enrich or modify objects, without modifying (or even rebuilding) the executable.
* Add "hotswap" to a project while developing
* Have complete runtime reflection by symbols or strings
* Messages:
    * Fast polymorphic calls &ndash; comparable to `std::function`
    * Have multicast messages, which are handled by many mixins within an object
    * Thread safe message calls &ndash; as thread safe as the underlying methods.

## Created with DynaMix

The following projects are known to use DynaMix as a key piece of their software architecture:

* [A3I](https://ibob.bg/blog/2022/01/25/what-we-do-at-viewray/) by ViewRay (Released 2022) - A medical system for cancer treatment
* [huse](https://github.com/iboB/huse) - a polymorphic serialization library
* [War Planet Online: Global Conquest](http://warplanetonline.com/) by Gameloft (Released 2017) - An MMORTS for Android and iOS.
* [Blitz Brigade: Rival Tactics](http://www.rival-tactics.com/) by Gameloft (Released 2017) - A multiplayer RTS for Android, iOS, and Windows.
* [Swords & Crossbones: An Epic Pirate Story](http://store.steampowered.com/app/383720/Swords__Crossbones_An_Epic_Pirate_Story/) by Epic Devs LLC (Released 2014) - A strategy/exploration game for Android, iOS, and PC.
* [Earthrise](https://www.play-earthrise.com/) by Masthead Studios (Released 2011) - An MMORPG (no longer supported). Uses an early proprietary prototype version of the library with a similar interface and a different implementation.

## Documentation

The documentation is part of the repo in the [doc/](doc/README.md) directory.

## Contributing

Contributions in the form of issues and pull requests are welcome.

## License

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

This software is distributed under the MIT Software License.

See accompanying file LICENSE or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2013-2023 [Borislav Stanimirov](http://github.com/iboB), [Zahary Karadjov](http://github.com/zah)

### Logo

[![License: CC BY 4.0](https://licensebuttons.net/l/by/4.0/80x15.png)](https://creativecommons.org/licenses/by/4.0/).

The [DynaMix logo](https://github.com/iboB/dynamix/tree/master/doc/logos) is licensed under a Creative Commons Attribution 4.0 International License. Copyright &copy; 2018 [area55git](https://github.com/area55git)

## V1

DynaMix v2 is a complete rewrite and though it has the same idea as v1, it is incompatible with it, and has a different interface, and different implementation for most features, and different terminology for some.

The last release of DynaMix v1 was [v1.4.0](https://github.com/iboB/dynamix/releases/tag/v1.4.0). The documentation is available [here](https://ibob.bg/dynamix/v1/index.html)

A list of the most notable differences between v1 and v2 can be found [here](doc/misc/v2-vs-v1.md).

## Boost.Mixin

DynaMix was initially developed as [Boost.Mixin](https://github.com/iboB/boost.mixin) but is now a separate library that doesn't depend on the Boost libraries Collection.

![Footer](https://s22.postimg.cc/gkvrylx9t/README_2.jpg)
