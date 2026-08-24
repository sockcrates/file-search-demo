# Doxygen documentation standard

Public C++ declarations in `src/` use Doxygen block comments placed immediately
before the declaration they describe. This project uses the standard
`/** ... */` form so the comments remain readable in code and render consistently
in Doxygen's HTML output.

Use this shape for public functions:

```cpp
/**
 * @brief One-sentence description in the present tense.
 *
 * Add behaviour, ownership, boundary, or performance detail when it helps a caller.
 * @param name Meaning and units of each non-obvious input.
 * @return Meaning of a non-void result, including sentinel values.
 * @throws Exception type and condition, when applicable.
 * @pre Required caller conditions, when applicable.
 * @complexity Time and space complexity for non-trivial operations.
 */
```

Document public types and their members, constructors, public functions, and externally meaningful
destructors. Every parameter and non-void result that needs interpretation has an `@param` or
`@return` entry. Prefer `@copydoc` for overrides whose contract is unchanged. Do not repeat obvious
implementation detail; document contracts, invariants, ownership, units, and edge cases instead.
Keep prose wrapped at 100 columns and use backticks for C++ identifiers and literal command names.

`@complexity` is a project alias that renders a `Complexity` paragraph. `docs/Doxyfile` treats
documentation warnings as errors and scans the headers exposed from `src/`. Configure CMake after
installing Doxygen, then run `cmake --build build --target docs` to generate the HTML reference.
