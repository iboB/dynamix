# Benchmark findings and todo

Findings from the revamped benchmarks, written so that each item can become an issue.

Numbers: Intel Core Ultra 9 285K pinned to a P-core, Ubuntu 24.04, GCC 13, release, shared lib. Instruction counts from `bench/cachegrind/baseline.json` (per iteration).

---

## Library performance

### 1. Mutations don't scale with threads

**Finding:** 4 threads, each mutating (add + remove a mixin) its own objects in a shared domain take the same wall time as 1 thread: 166 ns/op with 1 thread, 176 ns/op with 4. The C++ entity-component equivalent scales 2.4x (17 → 7 ns). Message calls scale fine (2.2x).

**Likely cause:** every mutation does a type lookup in `domain::impl::get_type(type_mutation&)`, which takes `m_type_registry.shared_lock()` (a `std::shared_mutex`). Taking a shared lock still writes to the mutex's state, so all threads write to the same cache line on every mutation (twice per add + remove). Not verified with a profiler.

**Investigate:**
- Confirm with `perf c2c` or `perf stat` on `bench-dynamix-threads --run-suite="parallel mutations"` (needs `kernel.perf_event_paranoid` <= 1).
- Options: cache the last resulting type per (object type, mutation) somewhere lock-free; make the type query cache read-mostly (RCU-like / copy-on-write snapshot with an atomic pointer); per-type cache of "mutation → type" edges (adding/removing a single mixin is by far the most common mutation); a lock-free hash map.

**Bench:** `threads` / "parallel mutations".

### 2. Mutations are expensive even single-threaded: the type lookup path

**Finding:** a mutation of an object to an existing type costs ~85 ns / ~2000 instructions. An add + remove pair is 4022 instructions vs 1388 for the entity-component equivalent (which also allocates). A recycling allocator barely helps (4022 → 3706 instr, 173 → 168 ns), so allocation is not the bottleneck. Creating an object with `mutate` costs 2833 instructions vs 1013 from a type: ~1800 instructions for building and resolving the mutation.

**Investigate:**
- Profile a single `mutate(obj, add<M>())` with callgrind (`--toggle-collect` or the same client requests) and see the split between building the `type_mutation` (pmr vector allocations?), the query lookup, `object_mutation`, constructing/moving mixins.
- The query cache is `pmr::map<type_query, const type*>` keyed on vectors of pointers: O(log n) vector comparisons per lookup. A hash map keyed on the mixin list would be O(1).
- `type_mutation` allocates its mixin vector. A small-buffer vector would avoid the allocation for typical sizes.
- Per-type cache of transitions ("this type + add M = that type") could skip the query lookup entirely for the common case.
- `domain::get_type(span)` has a `// TODO: optimize` comment: on a cache miss it creates a mutation which redoes the same work.

**Bench:** `object-lifetime` / "add and remove a component", "create objects"; `type-creation` / "type lookup".

### 3. Type lookup gets slower with the number of types

**Finding:** looking up an existing type costs 51 ns with 128 types, 73 ns with 1K, 154 ns with 16K (542 instructions at 1K). It doesn't depend on rules or features, only on the number of types.

**Cause:** the query cache is a `std::map` (O(log n), each comparison is a lexicographic comparison of pointer vectors, which also means cache misses on the map nodes).

**Investigate:** hash map for the query cache (see 2). Check if it also helps the single-threaded mutation cost.

**Bench:** `type-creation` / "type lookup".

### 4. Unicast message call: 50 instructions vs 19 for a virtual call, and 1.9x slower with cache misses

**Finding:** in cache, a unicast is ~1.25x a virtual call (7-8 ns vs 5-6 ns). With shuffled access to 1M objects it's 26 ns vs 14 ns (1.9x). Instruction count: 50 vs 19 (`std::function`: 22).

**Path (msg_caller.hpp `call_unicast`):** `obj.get_type()` → `t.ftable_at(info.id)` (load of `Msg::info.id`, bounds check, ftable entry) → `*fe.begin` → payload + mixin index → `obj.unchecked_get_at(index)` (object buffer) → indirect call to the generated thunk → method.

**Investigate:**
- Count the dependent loads on the critical path vs a virtual call and see which can be removed. The object buffer load is a likely extra cache miss: object → buffer → mixin, vs `unique_ptr` → object for virtual.
- Is `Msg::info` a function-local static with a guard or a plain global? Is `info.id` reloaded every call?
- Can the ftable entry store something that avoids a load (e.g. offset of the mixin in the buffer instead of an index)?
- Check the generated assembly of `call_unicast` for an `implements<>` message (thunk + method inlined into it?).

**Bench:** `msg-unicast`, `msg-call` / "call kinds".

### 5. `get<M>()` costs more than a message call

**Finding:** `obj.get<M>()->method()` is 67 instructions vs 45 for a message call doing the same. `get<M>()` alone: 57 instructions, ~5 ns. `has<M>()` is 37 instructions for what should be an index check.

**Cause:** `get<M>()` (object.hpp) → `g::get_mixin_info<M>()` (function-local static in `_dynamix_get_mixin_info`, guard check) → non-inline `object::get(const mixin_info&)` → `object::get(mixin_id)` → type index lookup → `safe_get_mixin`. Several non-inline calls across the shared library boundary.

**Investigate:** an inline fast path in the header for `has<M>()` / `get<M>()` (inline sparse index lookup in the type + `unchecked_get_at`). Consider whether `get_mixin_info<M>()` can avoid the static guard (e.g. a global instead of a function-local static).

**Bench:** `msg-call` / "queries", "call kinds".

### 6. Calling the next implementation costs more than a full message call

**Finding:** a message whose top implementation calls the next one with `DYNAMIX_CALL_NEXT_IMPL_MSG` is 109 instructions vs 45 for a plain call: the extra hop is ~64 instructions, more than a whole message call.

**Investigate:** `next_impl.hpp`: finding "this" mixin's position in the ftable entry is probably a linear search by mixin index. Check if it's avoidable (e.g. pass the ftable position).

**Bench:** `msg-call` / "call kinds" / `msg_next_impl`.

### 7. Multicast: 107 instructions vs 71 for a vector of virtual objects

**Finding:** for 2-6 implementers, multicast is 107 instructions per call vs 71 (virtual) and 80 (`std::function`). Wall time is close (memory layout helps), but the instruction overhead per implementer looks high.

**Investigate:** per-implementer cost in `call_multicast`: `call()` reloads the object buffer for every implementer, args are copied. Check the generated code.

**Bench:** `msg-multicast`.

### 8. Parallel type creation is slower than serial

**Finding:** creating 16K unique types from 4 threads takes 2.1 µs/type vs 0.8 µs from one thread (1K types: 6.0 vs 0.8 µs).

**Note:** type creation takes an exclusive lock, so no scaling is expected. Being 2.7-7x *slower* means heavy contention. Low priority (types are usually created early and rarely), but worth a look if anything is done for item 1. Rules are applied under the shared lock and then the exclusive lock is taken, so concurrent creators of different types serialize and bounce.

**Bench:** `threads` / "parallel type creation".

### 9. Per-mixin memory overhead: 24 bytes

**Finding:** each mixin in an object costs 24 bytes on top of its data. `sizeof(dynamix::object)` is 32. Presumably 8 of the 24 are the object pointer before the mixin and 16 are per-mixin data in the buffer (not verified).

**Investigate:** what the per-mixin data is and whether it can be reduced (e.g. offsets instead of pointers for internal mixins). Low priority unless objects with many tiny mixins are a target use case.

**Bench:** `memory`.

---

## Benchmark infrastructure

### 10. Verify the cachegrind baseline on CI

The baseline was generated locally (Ubuntu 24.04, GCC 13.3, valgrind 3.22), not on the CI runner. glibc selects `memcpy` & co based on CPU features, so counts may differ slightly. After the first CI run: if it fails with small uniform differences, replace `bench/cachegrind/baseline.json` with the `cachegrind-results` artifact.

### 11. Hybrid CPUs (P/E cores) make timings noisy

On the 285K, unpinned runs varied up to 2x (multicast especially). Pinned to a P-core (`taskset -c 3`) results are stable except "multicast: sequential access".

- Document in `bench/README.md`: pin with `taskset` on Linux (P-cores only for `threads`).
- Optionally: the shared `dbench::run` could print a warning, or an option to pin itself.
- Investigate the remaining noise in "multicast: sequential access".

### 12. picobench improvements

- `--iters=` etc. require a double dash (default prefix `-` + options which start with `-`). Single-dash `-iters=` is rejected as unknown. Either accept both or make the help clearer.
- Only the fastest sample is reported. A median and a spread (min/max or stddev) would make noise visible.
- Optional: JSON output for tooling.

### 13. Memory report in CI

`bench-dynamix-memory` is deterministic for a given platform. It could be compared against a committed expected output in the cachegrind job (same pinned environment) to catch memory regressions (object overhead, bytes per type, allocations per mutation).

### 14. Remove the dead `DYNAMIX_BUILD_COMPILER_BENCH` option

Defined in the root `CMakeLists.txt`, used nowhere.

### 15. Multi-threaded benchmarks on real hardware

The `threads` benchmark is excluded from CI (only built). Results are only meaningful with >= 4 physical cores. Consider a manual-only workflow on a bigger runner, or just document how to run it.

---

## Docs

### 16. "Comparable to `std::function`" claims

overview.md ("Fast polymorphic calls – comparable to `std::function`"), messages.md ("comparable to `std::function`"), and misc/dynamix-vs-x.md say message calls cost about as much as `std::function`. Measured: ~1.4x `std::function` in cache (50 vs 22 instructions), ~2.6x with cache misses at 1M objects. Either improve calls (item 4) or adjust the wording to "slower than a virtual call, in the same order of magnitude" and link to perf.md.

### 17. Thread safety page

`doc/README.md` links `working-with/thread-safety.md`, which doesn't exist. perf.md now also mentions that mutations of objects in the same domain don't scale. The page should document what's safe and what isn't, and the scaling limitation (item 1). To verify and document: calls are as thread-safe as the implementations (stated in messages.md); domain registration and type creation are synchronized (the domain uses shared mutexes); mutating an object concurrently with calls or other mutations on the same object is presumably not safe.

### 18. Update perf.md when the above improve

perf.md contains concrete numbers and the "mutations don't scale" known issue. Update after items 1-6.
