# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DynaMix is a C++17/C11 library for dynamic polymorphism and runtime object composition. It allows creating polymorphic objects from mixins at runtime, enabling features like dynamic type composition, runtime polymorphism, and live object mutation.

## Build System

This project uses CMake with presets for different configurations:

### Common Commands

```bash
# Configure and build (debug)
cmake --preset debug
cmake --build out/build/debug

# Configure and build (release)
cmake --preset release
cmake --build out/build/release

# Run tests
cmake --build out/build/debug --target test
# or
ctest --test-dir out/build/debug

# Run benchmarks
cmake --build out/build/release --target bench
# or run specific benchmark executables in out/build/release/bench/

# Build with sanitizers
cmake --preset debug-asan
cmake --build out/build/debug-asan

# Build with coverage
cmake --preset debug-coverage
cmake --build out/build/debug-coverage
```

### Build Options

Key CMake options (set during configure):
- `DYNAMIX_BUILD_TESTS=ON/OFF` - Enable/disable tests (default: ON in dev mode)
- `DYNAMIX_BUILD_EXAMPLES=ON/OFF` - Build examples (default: ON in dev mode)
- `DYNAMIX_BUILD_BENCH=ON/OFF` - Build benchmarks (default: ON in dev mode)
- `DYNAMIX_BUILD_V1COMPAT=ON/OFF` - Build v1 compatibility layer (default: ON in dev mode)
- `BUILD_SHARED_LIBS=ON/OFF` - Build shared libraries (default: ON)
- `DYNAMIX_STATIC=ON/OFF` - Force static build (default: OFF)

## Architecture Overview

### Core Components

1. **Domain** (`code/dynamix/domain.hpp`, `code/dnmx/domain.h`)
   - Central registry for mixins, features, and object types
   - Manages object type creation and lifecycle
   - Enforces mutation rules and handles garbage collection

2. **Object** (`code/dynamix/object.hpp`, `code/dnmx/object.h`)
   - Dynamic composition container holding multiple mixins
   - Supports runtime type mutation (adding/removing mixins)
   - Manages memory allocation for internal and external mixins

3. **Type** (`code/dynamix/type.hpp`, `code/dnmx/type.h`)
   - Immutable blueprint describing mixin combinations
   - Contains feature table (ftable) for efficient dispatch
   - Manages object buffer allocation strategies

4. **Mixin** (`code/dynamix/mixin_info.hpp`, `code/dnmx/mixin_info.h`)
   - Reusable components providing data and behavior
   - Basic building blocks for object composition
   - Can be internal (embedded) or external (separately allocated)

5. **Message System** (`code/dynamix/msg/`, `code/dnmx/msg/`)
   - Dynamic dispatch mechanism for polymorphic behavior
   - Supports unicast/multicast with bidding and chaining
   - Enables late-binding method calls across mixins

### API Layers

- **C++ API** (`code/dynamix/`): High-level, type-safe, RAII-based interface
- **C API** (`code/dnmx/`): Low-level, handle-based interface for language bindings

### Key Design Patterns

- **Registry Pattern**: Domain manages all components centrally
- **Strategy Pattern**: Features represent interchangeable algorithms
- **Composite Pattern**: Objects compose multiple mixins
- **Builder Pattern**: Fluent APIs for type mutations and object assembly

## Testing

- **Unit Tests**: Powered by [doctest](https://github.com/doctest/doctest) for C++ and [Unity](https://github.com/ThrowTheSwitch/Unity) for C
- **Test Location**: `test/` directory
- **Integration Tests**: `test/app-plugin/` demonstrates plugin architecture
- **Fuzzing Tests**: `test/fuzz-*.cpp` files for robustness testing

## Examples and Learning

- **Basic Usage**: `example/hello-world/` and `example/hello-c-world/`
- **Advanced Usage**: `test/app-plugin/` shows plugin system with shared libraries
- **Documentation**: `doc/` directory contains comprehensive guides
- **V1 Compatibility**: `v1compat/` provides migration layer from v1 to v2

## Development Workflow

1. **V1 vs V2**: This is DynaMix v2 - a complete rewrite incompatible with v1
2. **Benchmarks**: Use `bench/` directory for performance measurements
3. **Scratch Space**: `scratch/` directory for experiments and prototyping
4. **Memory Management**: Library supports `std::polymorphic_allocator` for custom allocation strategies
5. **Thread Safety**: Message calls are as thread-safe as underlying mixin methods

## Common Development Tasks

- **Adding New Mixins**: Use `DYNAMIX_DECLARE_MIXIN` and `DYNAMIX_DEFINE_MIXIN` macros
- **Defining Messages**: Use `DYNAMIX_DECLARE_MSG` and `DYNAMIX_DEFINE_MSG` macros
- **Type Mutations**: Use `dynamix::mutate()` with `add<>()` and `remove<>()` operations
- **Custom Domains**: Create isolated environments for different object systems
- **Plugin Development**: See `test/app-plugin/` for shared library patterns

## Dependencies

- **External**: itlib, splat (managed via CPM)
- **PMR Compatibility**: `code/dynamix/compat/pmr/` provides polymorphic memory resource support
- **Build Tools**: CMake 3.16+, Ninja (recommended), C++17 compiler, C11 compiler