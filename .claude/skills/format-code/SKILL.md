---
name: format-code
description: Run clang-format on src/ and tests/ via the CMake `format` target. Use when the user wants to format code, fix style, or check formatting before commit. Pass "check" to dry-run (fails on diff) instead of applying changes.
allowed-tools: Bash(cmake*)
---

## Format C++ sources

The root [CMakeLists.txt](../../../CMakeLists.txt) registers two custom targets when `clang-format` is on PATH:

- `format` — apply formatting in-place across `src/` and `tests/` (skips `tests/third_party/`)
- `format_check` — dry-run, fails with diff (use in CI / pre-commit)

Style configuration: [.clang-format](../../../.clang-format).

### Apply formatting

```bash
cmake --build build-desktop --target format
```

### Check only (no writes)

```bash
cmake --build build-desktop --target format_check
```

### Behavior based on arguments

- If `$ARGUMENTS` contains "check" → run `format_check`.
- Otherwise → run `format`.

### Prerequisites

- A configured `build-desktop/` directory. If missing, configure first:
  `cmake -S . -B build-desktop -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
- `clang-format` installed and on PATH. If absent, the targets won't be registered — re-configure CMake after installing.
