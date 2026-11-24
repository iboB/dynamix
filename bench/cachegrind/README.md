# Cachegrind Benchmarks

Wall-clock benchmarks can be too noisy and are not suitable to reliably detect regressions. We also use [Cachegrind](https://valgrind.org/docs/manual/cg-manual.html) to measure the number of instructions executed in single threaded benchmarks. Instruction counts are deterministic, so even a small change can be detected. CI also runs these benchmarks and compares the number of instructions with a baseline.

Note that insruction counts are not time. They don't reflect cache misses, banch mispredictions, or the cost of individual instructions (which on x86 can vary by a factor of 10). A change in the instruction count signal to look at, but not necessarily a problem.

## Requirements

* Linux with valgrind 3.22, including the valgrind headers (on Ubuntu these come in the `valgrind` package)
* Ruby 3.2 or later for the regression detection harness
* Build with `-DDYNAMIX_BENCH_CACHEGRIND=ON` to enable the instrumentation in the benchmarks themselves.

## Running Locally

```
cmake --preset release -DDYNAMIX_BENCH_CACHEGRIND=ON
cmake --build out/build/release --target cachegrind-bench-dynamix
```

The results are written to `out/build/release/bench/cachegrind-results.json`.

## Updating the Baseline

Instruction counts depend on the compiler version, the standard library, and may even depend on the host CPU, as the standard library may use different instructions depending on the CPU features. Therefore, the baseline must be produced on the same environment as the CI job.

When a change is expected to affect the instruction counts or when the CI environment changes:

* Run the cachegrind workflow either locally or on CI to produce a new baseline.
    * `$ cmake --build out/build/release --target cachegrind-bench-dynamix-update-baseline`
* Replace `baseline.json` with the new results
