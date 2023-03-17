# Differences Between DynaMix 1 and 2

* Multiple domain support
* No enforced canonical mixin order
* Mixins are internal by default
* General features
* Allocators replaced by `std::pmr::polymorphic_allocator`
* Mixins: more manual configuration
* Messages:
    * More verbose definitions
    * Inverse priority order (important for multicasts)
    * Combinators replaced by reducers
* Completely new features:
