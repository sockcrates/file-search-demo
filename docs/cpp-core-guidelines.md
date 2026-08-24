# C++ Core Guidelines compliance policy

This repository adopts the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
as its mandatory C++ design and implementation rules. This document is the local, normative interpretation
of those rules; `clang-tidy` is an enforcement aid, not proof of compliance by itself.

## Scope and required rules

The policy applies to all first-party C++ headers and translation units in `src/`, `tests/`, and
`benchmarks/`. Generated, third-party, and toolchain code is out of scope. Test code follows the same rules
as production code.

- **Interfaces and types (I.4, I.5, F.21, F.24):** use precise domain types; express bounded ranges with
  `std::span`; use `nullptr`, scoped enums, and explicit checked conversions. Document ownership, lifetime,
  preconditions, postconditions, and error behavior for public interfaces. Do not use sentinel, `bool`, or
  string-only error channels where a named result or error type is appropriate.
- **Errors (E.1, E.2, E.6, E.15, E.17):** each fallible boundary has a coherent and documented failure
  strategy. Error values and thrown error types carry a named category and the context callers need; failures
  are never silently discarded.
- **Classes and resources (C.2, C.20–C.22, C.35, R.1, R.5, R.11, R.12):** preserve invariants, prefer the
  rule of zero, use RAII, and never use owning raw pointers or manual resource-pairing. Polymorphic bases
  have virtual destructors.
- **Expressions and safety (ES.20, ES.23, ES.46–ES.48):** initialize values, avoid unchecked indexing,
  C-style casts, `const_cast`, and unsafe conversion. Boundary casts are kept small and justified.

Security-sensitive changes additionally review bounds, integer conversion, lifetime, error propagation, and
resource cleanup. Sanitizers complement these rules but never replace review or static analysis.

## Enforcement

`.clang-tidy` enables every available `cppcoreguidelines-*` check and treats all enabled diagnostics as
errors. It also enables analyzer, bug-prone, CERT, performance, portability, and selected readability checks.
Compiler diagnostics are errors and the normal build enables supported hardening.

`mise run check` is the required local gate: formatting, the Core Guidelines analysis, strict compilation,
and tests. `mise run sanitizer` is required for changes that affect runtime behavior. `clang-tidy` is an
explicit required mise tool pinned alongside LLVM 21. Run `mise install` after cloning or when
`mise.toml`/`mise.lock` changes. The default CMake configuration and the explicit `tidy` task fail when the
analyzer is unavailable, so it cannot silently pass the required gate.

`-DENABLE_CLANG_TIDY=OFF` is permitted only for a temporary local diagnostic or build investigation when
the analyzer itself prevents that investigation. It is not a compliance mode, cannot be used as CI or review
evidence, and does not waive this policy; rerun the default configuration and `mise run check` before
submitting the change.

## Exceptions

Exceptions are permitted only when a specific rule conflicts with correctness, portability, or a documented
interoperability boundary such as a POSIX C ABI. An exception must use the smallest
`NOLINTNEXTLINE(<check-name>)` or `NOLINT(<check-name>)`, and an adjacent comment containing:

1. An exception ID (`CG-###`), the exact check, and the relevant Core Guideline.
2. The safety argument, owner, and a removal condition.
3. A focused test when runtime behavior is affected.

Broad `NOLINT`, `NOLINTBEGIN`/`NOLINTEND`, disabled check families, warning suppressions, and undocumented
exceptions are prohibited.

The following configuration-scoped exceptions are approved and reviewed with `.clang-tidy`:

| ID | Check | Scope | Safety argument and removal condition |
| --- | --- | --- | --- |
| CG-001 | `cppcoreguidelines-pro-bounds-constant-array-index` | POSIX byte-processing implementation | Runtime-dependent offsets are checked against `std::span`/reader bounds. Remove when the analyzer can prove those range contracts. |
| CG-002 | `cppcoreguidelines-avoid-magic-numbers` | Conventional status, byte, and buffer constants | The whitelist avoids duplicate diagnostics where a named declaration already documents the value. Remove a value when it gains no such context. |

There are currently no approved source-code exceptions.
