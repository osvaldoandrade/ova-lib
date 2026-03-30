# Contributing to ova-lib

This guide keeps the contribution path short: build the library, run the tests, keep the docs aligned with the headers, and send one focused pull request at a time.

## Build

The project requires CMake 3.10 or newer and builds as C11. If your CMake build supports presets, use the preset path below.

```bash
cmake --preset dev
cmake --build build/dev
```

If you prefer the manual path:

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

## Test

Run the full suite through CTest:

```bash
ctest --test-dir build/dev --output-on-failure
```

If Valgrind was present when the build tree was configured, the `memcheck` target is also available:

```bash
cmake --build build/dev --target memcheck
```

## Code and Docs

Match the style already present in the touched module. Public C APIs use snake case. Enum values use upper snake case. Keep ownership rules explicit in headers and in prose.

If a change touches a public function, type, or return rule, update the matching page in `docs/` in the same pull request. The repo docs are the canonical manual. The wiki is a short map into that material.

## Tests for Changes

Add or update one focused test in `test/` for any behavior change. Each test file is a small standalone program registered in the `foreach(TEST ...)` loop in `CMakeLists.txt`.

## Pull Requests

One pull request should cover one logical change. Keep the description factual. Include the commands you ran, the docs you updated, and any gaps you left open on purpose.

## License

By contributing, you agree that your changes will be released under the [Apache 2.0 License](LICENSE).
