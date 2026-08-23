# find-exercise

A C++23 project built with CMake and LLVM Clang, managed through [mise](https://mise.jdx.dev/).

## Getting started

```sh
mise install
mise run build
mise run test
mise run lint
mise run format-check
```

`mise.toml` pins Clang/LLVM 21 and CMake 4. The build configuration explicitly selects `clang++` and produces `build/compile_commands.json` for editor tooling.

## Tasks

```sh
mise run configure
mise run build
mise run test
mise run lint
mise run format       # rewrite files using clang-format
mise run format-check # fail if formatting is needed
mise run check        # format-check, lint, and test
mise run clean
```

`lint` compiles every target with Clang's warnings enabled and treated as errors.
Formatting is defined in `.clang-format` and applied by the pinned `clang-format`
tool.
