# PMR Compatibility Layer

As of this writing libc++ still has no official release with pmr support. Even though 16 is soon to be released, it will be a while before it's widely available (I'm looking at you Apple). For this reason we introduce this compatibility layer which emulates `std::pmr`.

It is a minimalistic bare-bones reimplementation of std::pmr.

It does not use libc++'s `std::experimental::pmr` because it relies on externally provided `get_default_resource` which Dynamix chooses not to impose. As a bonus, the compatibility layer is usable even without `std::experimental::pmr` being available with `DYNAMIX_FORCE_PMR_COMPAT`
