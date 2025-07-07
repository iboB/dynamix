# Adding DynaMix to a Project

To build DynaMix you need a C++17-capable C++ compiler and a C11-capable C compiler. C++17 is required to use the C++ interface and C11 is required to use the C interface. MSVC 2022, gcc 9+, and clang 10+ are regularly tested and known to work.

Currently the only supported way to add DynaMix to a project is as a [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) package. Other ways are listed below, though they are not tested.

## As a CPM.cmake package

The recommended and easiest way to add the library is as a [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) package. If you're using this package manager (and you should be), you only need to add the package `iboB/dynamix` like `CPMAddPackage(gh:iboB/dynamix@2.0.2)`

## As a submodule/subrepo

DynaMix uses [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) to handle its own dependencies, but also bundles CPM.cmake itself. If you add DynaMix as a submodule of your repo and simply `add_subdirectory` it in CMake, everything will very likely work as intended. The CMake configuration of DynaMix will fetch the needed dependencies and everything will be configured properly.

The danger here is if your project already uses some of these dependencies. In such a case there will be a Clash of Targets™ and things will very likely not build (or worse yet, it will build but will contain strange bugs due to ODR violations and ABI differences).

The dependencies are:

* [iboB/splat](https://github.com/iboB/splat) `~> 1.3.1`
* [iboB/itlib](https://github.com/iboB/itlib) `~> 1.9.0`
* CMake only: [iboB/icm](https://github.com/iboB/icm) `~> 1.4.5`
* Tests only: [iboB/doctest-util](https://github.com/iboB/doctest-util) `~> 0.1.1`
* Tests only: [iboB/doctest-lib](https://github.com/iboB/doctest-lib) `~> 2.4.9a`
* C Tests only: [ThrowTheSwitch/Unity](https://github.com/iboB/doctest-util) `~> 2.5.2`
* Benchmarks only: [iboB/picobench](https://github.com/iboB/picobench) `~> 2.04`

## More

As a whole, the build of DynaMix is pretty straightforward. The core dependencies are header-only. So if you fetch them and build all .c and .cpp files in `/code` into a library, everything should be OK.

Creating build scripts for the library with any build system should be easy.

The tests and benchmarks involve a bit more CMake scripting, but their build is mostly straightforward as well.

The v1compat library generates code through a Ruby script and thus its build is a bit more involved, but new users should not be using v1compat anyway.
