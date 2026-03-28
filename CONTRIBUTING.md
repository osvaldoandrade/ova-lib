# Contributing to ova-lib

Thank you for your interest in contributing to ova-lib! This guide covers
everything you need to get started.

## Table of Contents

- [Getting Started](#getting-started)
- [Build Instructions](#build-instructions)
- [Running Tests](#running-tests)
- [Code Style Guidelines](#code-style-guidelines)
- [Adding a New Data Structure](#adding-a-new-data-structure)
- [Test Requirements](#test-requirements)
- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)
- [Pull Request Process](#pull-request-process)
- [License](#license)

## Getting Started

1. Fork the repository on GitHub.
2. Clone your fork locally:
   ```bash
   git clone https://github.com/<your-username>/ova-lib.git
   cd ova-lib
   ```
3. Create a branch for your change:
   ```bash
   git checkout -b my-feature
   ```

## Build Instructions

ova-lib uses **CMake** (>= 3.10) and targets the **C11** standard.

### Using CMake presets (CMake 3.21+)

```bash
cmake --preset dev
cmake --build build/dev
```

### Manual build

```bash
mkdir build && cd build
cmake ..
make
```

Both a static (`libova_lib.a`) and a shared (`libova_lib.so`) library are
produced under `build/lib`.

## Running Tests

After building, run the full test suite with:

```bash
ctest --test-dir build/dev --output-on-failure
```

Or from inside the build directory:

```bash
ctest --output-on-failure
```

If Valgrind is available, you can run the suite under memcheck:

```bash
make memcheck
```

All tests must pass before submitting a pull request.

## Code Style Guidelines

### Language and standard

- Write **ANSI C** targeting the **C11** standard.
- Keep the code portable across POSIX systems.

### Naming conventions

| Element            | Convention    | Example                          |
|--------------------|---------------|----------------------------------|
| Functions          | `snake_case`  | `queue_enqueue`, `create_list`   |
| Structs / typedefs | `snake_case`  | `queue_entry`, `map_entry`       |
| Enum type names    | `snake_case`  | `queue_type`, `tree_type`        |
| Enum values        | `UPPER_CASE`  | `QUEUE_TYPE_NORMAL`, `AVL_TREE`  |
| Constants / macros | `UPPER_CASE`  | `INITIAL_CAPACITY`, `LOAD_FACTOR`|

### Function naming pattern

- **Factory functions**: `create_{type}()` — e.g. `create_queue()`, `create_tree()`
- **Operations**: `{type}_{operation}()` — e.g. `graph_add_vertex()`, `bloom_filter_add()`
- **Destroy / free**: `{type}_free()` or `{type}_destroy()`

### Documentation

- Document every public API function with Doxygen-style comments in the
  corresponding header file:
  ```c
  /**
   * @brief Short description of the function.
   *
   * Longer explanation if needed.
   *
   * @param param_name Description of parameter.
   * @return Description of return value.
   */
  ```

### Memory management

- Always check the return value of `malloc` / `realloc` before using the
  pointer. Store `realloc` results in a temporary variable to avoid leaking
  memory on failure.

## Adding a New Data Structure

ova-lib uses a **factory pattern** so that callers work through an abstract
interface. Follow these steps when adding a new data structure:

1. **Create a public header** in `include/` (e.g. `include/my_struct.h`).
   - Define an opaque struct with function-pointer fields for each operation.
   - Declare a factory function (`create_my_struct()`) that returns a pointer
     to the struct.

2. **Add implementation files** under `src/my_struct/`.
   - Implement each operation as a static function.
   - Wire the function pointers in the factory function.

3. **Include the new header** in `include/ova.h` so it is available through the
   umbrella header.

4. **Register source files** in `CMakeLists.txt` — add them to both the
   `ova_lib_static` and `ova_lib_shared` library targets.

5. **Write tests** in `test/test_my_struct.c` using the existing test harness
   (`base_test.h`). Register the test executable in `CMakeLists.txt` by
   appending its name to the `foreach(TEST ...)` loop.

6. **Update documentation** — add a note in `README.md` and, if appropriate,
   create a page under `docs/`.

## Test Requirements

- **Every change must include tests.** New data structures and bug fixes alike
  require accompanying unit tests.
- Tests use the project's custom harness defined in `test/base_test.h`. Use
  helpers such as `print_test_result()`, `assert_int_equal()`,
  `assert_not_null()`, and `assert_string_equal()`.
- Each test file should be self-contained with a `main()` that runs all of its
  test cases.
- All existing tests must continue to pass.

## Reporting Bugs

Open a GitHub issue with the following information:

1. **Summary** — a short description of the problem.
2. **Steps to reproduce** — minimal code or commands that trigger the issue.
3. **Expected behavior** — what you expected to happen.
4. **Actual behavior** — what actually happened (include any error output).
5. **Environment** — OS, compiler version, and CMake version.

## Requesting Features

Open a GitHub issue describing:

1. **Use case** — explain the problem the feature would solve.
2. **Proposed solution** — describe the API or behavior you have in mind.
3. **Alternatives considered** — mention any workarounds you explored.

Feature requests that include a proof-of-concept implementation are especially
welcome.

## Pull Request Process

1. **One logical change per PR.** Keep pull requests focused; split unrelated
   changes into separate PRs.
2. **Branch from `main`** and keep your branch up to date.
3. **Ensure CI passes.** The GitHub Actions workflow builds the library on
   Ubuntu (x86 and x64) and macOS (x64) and runs the full test suite. All
   checks must be green before a PR can be merged.
4. **Write a clear description.** Explain *what* the change does and *why* it is
   needed.
5. **Be responsive to reviews.** A maintainer will review your PR. Please
   address any feedback promptly.

## License

By contributing to ova-lib you agree that your contributions will be licensed
under the [Apache 2.0 License](LICENSE).
