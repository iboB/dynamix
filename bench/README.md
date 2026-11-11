# DynaMix Benchmarks

Powered by [picobench](https://github.com/iboB/picobench)

| Benchmark | What it measures |
|-----------|------------------|
| [msg-unicast](msg-unicast/) | Unicast message calls vs virtual functions and `std::function`, with sequential and shuffled access to the objects |
| [msg-multicast](msg-multicast/) | Multicast message calls vs vectors of virtual objects and `std::function`-s, with sequential and shuffled access |
| [msg-call](msg-call/) | Kinds of calls (direct, virtual, message, getting a mixin, calling the next implementation), default implementations, and queries (`has`, `get`, `implements`, type classes) |
| [object-lifetime](object-lifetime/) | Creating, copying, destroying, and mutating objects vs a C++ class and the entity-component pattern, with custom allocators, and internal vs external mixins |
| [type-creation](type-creation/) | Creating new types and looking up existing ones |
| [threads](threads/) | Scaling of message calls, mutations, and type creation with multiple threads |
| [memory](memory/) | Not a timing benchmark: a report of the memory used by objects and types, and the allocations of common operations |

Each benchmark has a target `run-bench-dynamix-<name>`, or the executable can be run directly. Note that picobench command line options use a double dash. For example:

```
bench-dynamix-msg-unicast --iters=1024,1048576 --samples=10
bench-dynamix-msg-call --run-suite=queries
```

There's also a `smoke-bench-dynamix` target which runs all single-threaded benchmarks with a few iterations and a single sample. It's used in CI to check that the benchmarks work and produce consistent results.

For regression testing with instruction counts, see [cachegrind](cachegrind/README.md).

The data for each benchmark is generated with a fixed seed and the random utilities in [common](common/), which produce the same data with every standard library.

Results are only comparable when produced on the same machine. Use a quiet machine (no other load, fixed CPU frequency if possible) for meaningful results. The "Dim" column is the number of objects or operations, and picobench reports the fastest of several samples.
