# Cachegrind Benchmarks

Wall-clock benchmark results on CI machines are too noisy to detect regressions. Instead, CI runs the single-threaded benchmarks with [cachegrind](https://valgrind.org/docs/manual/cg-manual.html) and compares the number of executed instructions with a baseline. Instruction counts are deterministic, so even small changes are detected.

Note that instruction counts are not time. They don't reflect cache misses, branch mispredictions, or the cost of different instructions. A change in the instruction count is a signal to look at, not a measurement of performance. Use the regular benchmarks on a quiet machine to measure time.

## How it works

* When configured with `DYNAMIX_BENCH_CACHEGRIND=ON`, the benchmarks enable cachegrind instrumentation only for their measured part (see `dbench::scope` in [bench-main.hpp](../common/bench-main.hpp)).
* [cachegrind.rb](cachegrind.rb) runs each benchmark separately under `valgrind --tool=cachegrind --instr-at-start=no` with a single sample and 1024 iterations, and collects the instruction count.
* The counts are compared to [baseline.json](baseline.json). An increase of more than 2% for any benchmark is a regression and fails the run.

The multi-threaded benchmarks are not included.

## Requirements

Linux with valgrind 3.22 or later (with its headers, which are in the `valgrind` package on Ubuntu), and Ruby.

## Running locally

```
cmake --preset release-gcc -B build -DDYNAMIX_BENCH_CACHEGRIND=ON
cmake --build build --target cachegrind-bench-dynamix
```

The results are written to `build/bench/cachegrind-results.json`.

## Updating the baseline

Instruction counts depend on the compiler, the standard library, and valgrind (and even on the CPU, as the standard library may choose different implementations of functions like `memcpy` based on it). The baseline must be produced in the same environment as the CI job: `ubuntu-24.04` with its default GCC and valgrind.

When a change is expected to affect the instruction counts (or when the CI environment changes):

1. Run the [Cachegrind workflow](../../.github/workflows/cachegrind.yml) on the branch (it can be started manually).
2. Download the `cachegrind-results` artifact and replace `baseline.json` with it.
3. Commit the new baseline with the change.

On a machine with the same environment, `cmake --build build --target cachegrind-bench-dynamix-update-baseline` overwrites the baseline directly.
